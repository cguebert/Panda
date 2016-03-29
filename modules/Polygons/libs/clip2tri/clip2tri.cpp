/*
 * clip2tri.h
 *
 * Authors: kaen, raptor, sam686, watusimoto
 *
 * Originally from the bitfighter source code
 */

// Modified by Christophe Guébert to also sanitize the enclosing polygon

#include "clip2tri.h"
#include "../poly2tri/poly2tri.h"

#include <cstdio>
#include <limits>

using namespace std;
using namespace p2t;
using namespace ClipperLib;

namespace c2t
{

static const F32 CLIPPER_SCALE_FACT = 1000.0f;
static const F32 CLIPPER_SCALE_FACT_INVERSE = 0.001f;

Path upscaleClipperPoints(const vector<Point> &inputPolygon)
{
   Path outputPolygon;
   outputPolygon.reserve(inputPolygon.size());

   auto maxVal = std::numeric_limits<ClipperLib::cInt>::max();
   IntPoint prevPt(maxVal, maxVal);
   for (const auto& pt : inputPolygon)
   {
		IntPoint newPt(S64(pt.x * CLIPPER_SCALE_FACT), S64(pt.y * CLIPPER_SCALE_FACT));
		if (newPt == prevPt)
			continue;
		outputPolygon.push_back(newPt);
		prevPt = newPt;
   }

   if (outputPolygon.size() > 1 && outputPolygon.front() == outputPolygon.back())
	   outputPolygon.pop_back();

   return outputPolygon;
}


Paths upscaleClipperPoints(const vector<vector<Point> > &inputPolygons)
{
   Paths outputPolygons;
   outputPolygons.reserve(inputPolygons.size());
   for (const auto& input : inputPolygons)
	   outputPolygons.push_back(upscaleClipperPoints(input));

   return outputPolygons;
}


// Use Clipper to merge inputPolygons, placing the result in a Polytree
// NOTE: this does NOT downscale the Clipper points.  You must do this afterwards
//
// Here you add all your non-navigatable objects (e.g. walls, barriers, etc.)
bool mergePolysToPolyTree(const vector<vector<Point> > &inputPolygons, PolyTree &solution)
{
   Paths input = upscaleClipperPoints(inputPolygons);

   // Fire up clipper and union!
   Clipper clipper;
   clipper.StrictlySimple(true);

   try  // there is a "throw" in AddPolygon
   {
      clipper.AddPaths(input, ptSubject, true);
   }
   catch(...)
   {
      printf("clipper.AddPaths, something went wrong\n");
   }

   return clipper.Execute(ctUnion, solution, pftNonZero, pftNonZero);
}

// Shrink large polygons by reducing each coordinate by 1 in the
// general direction of the last point as we wind around
//
// This normally wouldn't work in every case, but our upscaled-by-1000 polygons
// have little chance to create new duplicate points with this method.
//
// For information on why this was needed, see:
//
//    https://code.google.com/p/poly2tri/issues/detail?id=90
//
static void edgeShrink(Path &path)
{
   U32 prev = (U32)path.size() - 1;
   for(U32 i = 0; i < path.size(); i++)
   {
      // Adjust coordinate by 1 depending on the direction
      path[i].X - path[prev].X > 0 ? path[i].X-- : path[i].X++;
      path[i].Y - path[prev].Y > 0 ? path[i].Y-- : path[i].Y++;

      prev = i;
   }
}


// This uses poly2tri to triangulate.  poly2tri isn't very robust so clipper needs to do
// the cleaning of points before getting here.
//
// A tree structure of polygons is required for doing complex polygons-within-polygons.
// For reference discussion on how this started to be developed, see here:
//
//    https://code.google.com/p/poly2tri/issues/detail?id=74
//
// For assistance with a special case crash, see this utility:
//    http://javascript.poly2tri.googlecode.com/hg/index.html
//
vector<Point> triangulateComplex(const PolyTree &polyTree)
{ 
   vector<Point> outputTriangles;

   // Traverse our polyline nodes and triangulate them with only their children holes
   PolyNode *currentNode = polyTree.GetFirst();
   while(currentNode != NULL)
   {
      if(!currentNode->IsHole())
      {
		 // Keep track of memory for all the poly2tri objects we create
		 vector<vector<p2t::Point*> > linesRegistry;

		 // Build up this polyline in poly2tri's format
         vector<p2t::Point*> polyline;
         for(U32 j = 0; j < currentNode->Contour.size(); j++)
            polyline.push_back(new p2t::Point(F64(currentNode->Contour[j].X), F64(currentNode->Contour[j].Y)));

         linesRegistry.push_back(polyline);  // Memory

         // Set our polyline in poly2tri
         p2t::CDT cdt(polyline);

         for(U32 j = 0; j < currentNode->Childs.size(); j++)
         {
            PolyNode *childNode = currentNode->Childs[j];

            // Slightly modify the polygon to guarantee no duplicate points
            edgeShrink(childNode->Contour);

            vector<p2t::Point*> hole;
            for(U32 k = 0; k < childNode->Contour.size(); k++)
               hole.push_back(new p2t::Point(F64(childNode->Contour[k].X), F64(childNode->Contour[k].Y)));

            linesRegistry.push_back(hole);  // Memory

            // Add the holes for this polyline
            cdt.AddHole(hole);
         }

         cdt.Triangulate();

         // Add current output triangles to our total
         vector<p2t::Triangle*> currentOutput = cdt.GetTriangles();

         // Downscale Clipper points, copy our data to TNL::Point and to our output Vector
         p2t::Triangle *currentTriangle;
         for(U32 j = 0; j < currentOutput.size(); j++)
         {
            currentTriangle = currentOutput[j];
            outputTriangles.push_back(Point(currentTriangle->GetPoint(0)->x * CLIPPER_SCALE_FACT_INVERSE, currentTriangle->GetPoint(0)->y * CLIPPER_SCALE_FACT_INVERSE));
            outputTriangles.push_back(Point(currentTriangle->GetPoint(1)->x * CLIPPER_SCALE_FACT_INVERSE, currentTriangle->GetPoint(1)->y * CLIPPER_SCALE_FACT_INVERSE));
            outputTriangles.push_back(Point(currentTriangle->GetPoint(2)->x * CLIPPER_SCALE_FACT_INVERSE, currentTriangle->GetPoint(2)->y * CLIPPER_SCALE_FACT_INVERSE));
         }

		 // Clean up memory used with poly2tri
		 // Free the polylines and holes
		 for (auto& line : linesRegistry)
		 {
			 for(auto pt : line)
				 delete pt;
			 line.clear();
		 }
      }

      currentNode = currentNode->GetNext();
   }

   return outputTriangles;
}

vector<Point> triangulate(const vector<vector<Point>>& inputPolygons)
{
   // Use clipper to clean.  This upscales the floating point input
   PolyTree polyTree;
   mergePolysToPolyTree(inputPolygons, polyTree);

   // This will downscale the Clipper output and use poly2tri to triangulate
   return triangulateComplex(polyTree);
}

} /* namespace c2t */
