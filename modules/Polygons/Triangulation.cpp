#include <panda/document/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/Polygon.h>
#include <panda/types/Mesh.h>

#include "ClipperUtils.h"
#include <modules/Polygons/libs/poly2tri/poly2tri.h>

#include <iostream>

// Using some code from clip2tri (bitfighter source code)

namespace panda {

using types::Mesh;
using types::Path;
using types::Point;
using types::Polygon;

inline Point convert(const p2t::Point pt)
{
	return Point(static_cast<float>(pt.x) * clipperToPandaFactor
		, static_cast<float>(pt.y) * clipperToPandaFactor); 
}

class Polygon_Triangulation : public PandaObject
{
public:
	PANDA_CLASS(Polygon_Triangulation, PandaObject)

	Polygon_Triangulation(PandaDocument *doc)
		: PandaObject(doc)
		, m_input(initData("input", "Input polygon"))
		, m_output(initData("output", "Output mesh"))
	{
		addInput(m_input);

		addOutput(m_output);
	}

	// Shrink large polygons by reducing each coordinate by 1 in the 
	// general direction of the last point as we wind around
	//
	// This normally wouldn't work in every case, but our upscaled-by-1000 polygons
	// have little chance to create new duplicate points with this method.
	//
	// For information on why this was needed, see:
	//    https://code.google.com/p/poly2tri/issues/detail?id=90
	//
	static void edgeShrink(ClipperLib::Path &path)
	{
		auto prev = path.size() - 1;
		for(size_t i = 0; i < path.size(); i++)
		{
			// Adjust coordinate by 1 depending on the direction
			path[i].X - path[prev].X > 0 ? path[i].X-- : path[i].X++;
			path[i].Y - path[prev].Y > 0 ? path[i].Y-- : path[i].Y++;

			prev = i;
		}
	}

	void triangulate(const Polygon& inputPoly, std::vector<Mesh>& outputMeshes)
	{
		// Use Clipper to ensure strictly simple polygons
		using namespace ClipperLib;
		Clipper clipper;
		clipper.StrictlySimple(true);

		try
		{ 
			clipper.AddPaths(polyToClipperPaths(inputPoly), ptSubject, true);
		}
		catch (...)
		{
			std::cerr << "Error in Clipper::AddPaths" << std::endl;
			return;
		}

		PolyTree polyTree;
		clipper.Execute(ctUnion, polyTree, pftNonZero, pftNonZero);
		
		// Traverse the PolyTree nodes and triangulate them with only their children holes
		PolyNode *currentNode = polyTree.GetFirst();
		while(currentNode != NULL)
		{
			// The holes are only used as the children of the contours
			if (currentNode->IsHole())
			{
				currentNode = currentNode->GetNext();
				continue;
			}

			// Convert to poly2tri's format
			std::vector<p2t::Point> allPoints;
			std::vector<size_t> linesSizes;

			// Contour
			for (const auto& pt : currentNode->Contour)
				allPoints.emplace_back(static_cast<double>(pt.X), static_cast<double>(pt.Y));
			linesSizes.push_back(allPoints.size());

			// Holes
			for(const auto childNode : currentNode->Childs)
			{
				// Slightly modify the polygon to guarantee no duplicate points
				edgeShrink(childNode->Contour);

				for (const auto& pt : childNode->Contour)
					allPoints.emplace_back(static_cast<double>(pt.X), static_cast<double>(pt.Y));
				linesSizes.push_back(allPoints.size());
			}

			// Poly2Tri uses pointers to points
			std::vector<p2t::Point*> contourLine;
			std::vector<std::vector<p2t::Point*>> holes;

			auto nbLines = linesSizes.size();
			size_t start = 0;
			for (size_t i = 0; i < nbLines; ++i)
			{
				size_t end = linesSizes[i];
				std::vector<p2t::Point*> line;
				line.reserve(end - start);
				for (size_t j = start; j < end; ++j)
					line.push_back(&allPoints[j]);
				start = end;

				if (!i) contourLine = std::move(line);
				else	holes.push_back(std::move(line));
			}

			// Set the contour in poly2tri
			p2t::CDT cdt(contourLine);

			// Add the holes
			for(auto& hole : holes)
				cdt.AddHole(hole);

			// Do the actual triangulation
			cdt.Triangulate();

			// Downscale the points and add them to the mesh
			Mesh mesh;
			for (const auto pt : allPoints)
				mesh.addPoint(convert(pt));

			// Convert the triangles
			auto triangles = cdt.GetTriangles();
			auto firstPt = allPoints.data();
			for (const auto triangle : triangles)
			{
				int ptId1 = std::distance(firstPt, triangle->GetPoint(0));
				int ptId2 = std::distance(firstPt, triangle->GetPoint(1));
				int ptId3 = std::distance(firstPt, triangle->GetPoint(2));
				mesh.addTriangle(ptId1, ptId2, ptId3);
			}
			outputMeshes.push_back(mesh);

			currentNode = currentNode->GetNext();
		}
	}

	void update()
	{
		const auto& input = m_input.getValue();
		auto output = m_output.getAccessor();
		auto& outputMeshes = output.wref();
		outputMeshes.clear();

		if(input.empty())
			return;

		for(const auto& inputPoly : input)
			triangulate(inputPoly, outputMeshes);
	}

protected:
	Data< std::vector<Polygon> > m_input;
	Data< std::vector<Mesh> > m_output;
};

int Polygon_TriangulationClass = RegisterObject<Polygon_Triangulation>("Generator/Mesh/Triangulation").setDescription("Compute a triangulation of a polygon");

} // namespace Panda


