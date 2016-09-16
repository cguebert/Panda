#include <panda/object/ObjectFactory.h>
#include <panda/graphics/Image.h>
#include <panda/types/ImageWrapper.h>
#include <panda/types/Mesh.h>
#include <panda/types/Polygon.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4244) /* conversion from 'type1' to 'type2', possible loss of data */
#endif

#define PAR_MSQUARES_IMPLEMENTATION
#include "par_msquares.h"

#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace
{

	struct marching_context
	{
		int width = 0, height = 0, nb = 0;
		int threshold = 128;
		unsigned char* data;
	};

	int isInside(int location, void* ptr)
	{
		marching_context* context = static_cast<marching_context*>(ptr);
		if (location < 0 || location > context->nb)
			return 0;
		auto data = &context->data[location * 4];
		auto val = (panda::graphics::gray(data) * panda::graphics::alpha(data)) >> 8;
		return val  > context->threshold;
	}

	float value(float x, float y, void* ptr)
	{
		marching_context* context = static_cast<marching_context*>(ptr);
		int i = PAR_CLAMP(static_cast<int>(context->width * x), 0, context->width - 1);
		int j = PAR_CLAMP(static_cast<int>(context->height * y), 0, context->height - 1);
		auto data = &context->data[(i + j * context->width) * 4];
		auto val = (panda::graphics::gray(data) * panda::graphics::alpha(data)) >> 8;
		return static_cast<float>(val);
	}
}

namespace panda {

using types::ImageWrapper;
using types::Mesh;
using types::Path;
using types::Polygon;
using types::Point;

class GeneratorMesh_MarchingSquares : public PandaObject
{
public:
	PANDA_CLASS(GeneratorMesh_MarchingSquares, PandaObject)

	GeneratorMesh_MarchingSquares(PandaDocument *doc)
		: PandaObject(doc)
		, m_image(initData("image", "Input image"))
		, m_cellSize(initData(5, "cellSize", "Size of the cell for the marching cube"))
		, m_threshold(initData(128, "threshold", "Keep points whose value is bigger than this threshold"))
		, m_inverse(initData(0, "inverse", "Inverse the selection"))
		, m_meshes(initData("mesh", "Mesh created from the marching squares"))
	{
		addInput(m_image);
		addInput(m_cellSize);
		addInput(m_threshold);
		addInput(m_inverse);

		addOutput(m_meshes);

		m_inverse.setWidget("checkbox");
	}

	void update()
	{
		auto outMeshes = m_meshes.getAccessor();
		outMeshes.clear();

		// Get the image
		panda::graphics::Image image;
		{
			helper::ScopedEvent log("Get image", this);
			image = m_image.getValue().getImage();
			if (!image)
				return;
		}

		// Do the marching squares
		auto size = image.size();
		auto w = size.width(), h = size.height();

		marching_context context;
		context.width = w;
		context.height = h;
		context.nb = w * h;
		context.data = image.data();
		context.threshold = m_threshold.getValue();
		int flags = PAR_MSQUARES_SIMPLIFY;
		if (m_inverse.getValue() != 0)
			flags |= PAR_MSQUARES_INVERT;
		auto mlist = par_msquares_function(w, h, m_cellSize.getValue(), flags, &context, isInside, value);

		// Convert to panda meshes
		int m = std::max(w, h);
		int nbMeshes = par_msquares_get_count(mlist);
		for (int i = 0; i < nbMeshes; ++i)
		{
			auto mesh = par_msquares_get_mesh(mlist, i);
			Mesh outMesh;
			
			auto pointsPtr = mesh->points;
			for (int j = 0; j < mesh->npoints; ++j)
			{
				outMesh.addPoint(Point(pointsPtr[0] * m, h - 1 - pointsPtr[1] * m));
				pointsPtr += mesh->dim;
			}

			auto triPtr = mesh->triangles;
			for (int j = 0; j < mesh->ntriangles; ++j)
			{
				outMesh.addTriangle(triPtr[0], triPtr[1], triPtr[2]);
				triPtr += 3;
			}

			outMeshes.push_back(outMesh);
		}

		par_msquares_free(mlist);
	}

protected:
	Data<ImageWrapper> m_image;
	Data<int> m_cellSize, m_threshold, m_inverse;
	Data<std::vector<Mesh>> m_meshes;
};

int GeneratorMesh_MarchingSquaresClass = RegisterObject<GeneratorMesh_MarchingSquares>("Generator/Mesh/Marching squares")
		.setName("Marching squares mesh")
		.setDescription("Create a mesh by extracting a region of an image");

//****************************************************************************//

class GeneratorPolygon_MarchingSquares : public PandaObject
{
public:
	PANDA_CLASS(GeneratorPolygon_MarchingSquares, PandaObject)

	GeneratorPolygon_MarchingSquares(PandaDocument *doc)
		: PandaObject(doc)
		, m_image(initData("image", "Input image"))
		, m_cellSize(initData(5, "cellSize", "Size of the cell for the marching cube"))
		, m_threshold(initData(128, "threshold", "Keep points whose value is bigger than this threshold"))
		, m_polygons(initData("polygon", "Polygon created from the marching squares"))
	{
		addInput(m_image);
		addInput(m_cellSize);
		addInput(m_threshold);

		addOutput(m_polygons);
	}

	void update()
	{
		auto acc = m_polygons.getAccessor();
		auto& outPolygons = acc.wref();
		outPolygons.clear();

		// Get the image
		panda::graphics::Image image;
		{
			helper::ScopedEvent log("Get image", this);
			image = m_image.getValue().getImage();
			if (!image)
				return;
		}

		// Do the marching squares
		auto size = image.size();
		auto w = size.width(), h = size.height();

		marching_context context;
		context.width = w;
		context.height = h;
		context.nb = w * h;
		context.data = image.data();
		context.threshold = m_threshold.getValue();
		int flags = 0;
		auto mlist = par_msquares_function(w, h, m_cellSize.getValue(), flags, &context, isInside, value);

		// Convert to panda polygons
		std::vector<Path> holes;
		int m = std::max(w, h);
		int nbMeshes = par_msquares_get_count(mlist);
		for (int i = 0; i < nbMeshes; ++i)
		{
			auto mesh = par_msquares_get_mesh(mlist, i);
			auto poly = par_msquares_extract_boundary(mesh);
			
			for (int j = 0; j < poly->nchains; ++j)
			{
				auto pts = poly->chains[j];
				Path path;
				for (int k = 0; k < poly->lengths[j]; ++k)
					path.points.push_back(Point(pts[k * 2] * m, h - 1 - pts[k * 2 + 1] * m));

				if (areaOfPolygon(path) < 0)
				{
					reorientPolygon(path);
					Polygon outPoly;
					outPoly.contour = std::move(path);
					outPolygons.push_back(std::move(outPoly));
				}
				else if (!path.points.empty())
				{
					if (!outPolygons.empty() && polygonContainsPoint(outPolygons.back().contour, path.points[0]))
						outPolygons.back().holes.push_back(std::move(path));
					else
						holes.push_back(std::move(path));
				}
			}
		}

		// Find the correct polygon to put holes in
		std::vector<Path> orphans;
		for (auto& hole : holes)
		{
			bool found = false;
			for (auto& poly : outPolygons)
			{
				if (polygonContainsPoint(poly.contour, hole.points[0]))
				{
					poly.holes.push_back(std::move(hole));
					found = true;
					break;
				}
			}

			if (!found)
				orphans.push_back(std::move(hole));
		}

		// If we really didn't find where to place that path, we create a new polygon for it
		for (auto& orphan : orphans)
		{
			Polygon poly;
			poly.contour = std::move(orphan);
			outPolygons.push_back(std::move(poly));
		}

		par_msquares_free(mlist);
	}

protected:
	Data<ImageWrapper> m_image;
	Data<int> m_cellSize, m_threshold;
	Data<std::vector<Polygon>> m_polygons;
};

int GeneratorPolygon_MarchingSquaresClass = RegisterObject<GeneratorPolygon_MarchingSquares>("Generator/Polygon/Marching squares")
		.setName("Marching squares polygon")
		.setDescription("Create a polygon by extracting a region of an image");
} // namespace Panda
