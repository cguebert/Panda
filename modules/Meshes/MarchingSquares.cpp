#include <panda/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/graphics/Image.h>
#include <panda/types/ImageWrapper.h>
#include <panda/types/Mesh.h>

#define PAR_MSQUARES_IMPLEMENTATION
#include "par_msquares.h"

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
		int i = PAR_CLAMP(context->width * x, 0, context->width - 1);
		int j = PAR_CLAMP(context->height * y, 0, context->height - 1);
		auto data = &context->data[(i + j * context->width) * 4];
		return (panda::graphics::gray(data) * panda::graphics::alpha(data)) >> 8;
	}
}

namespace panda {

using types::ImageWrapper;
using types::Mesh;
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
		, m_meshes(initData("mesh", "Mesh created from the marching squares"))
	{
		addInput(m_image);
		addInput(m_cellSize);
		addInput(m_threshold);

		addOutput(m_meshes);
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

		auto size = image.size();
		auto w = size.width(), h = size.height();
		int nb = w * h;
		auto data = image.data();

		// Do the marching squares
		marching_context context;
		context.width = w;
		context.height = h;
		context.nb = w * h;
		context.data = image.data();
		context.threshold = m_threshold.getValue();
		int flags = PAR_MSQUARES_SIMPLIFY;
		auto mlist = par_msquares_function(w, h, m_cellSize.getValue(), flags, &context, isInside, value);

		// Convert to panda meshes
		float m = std::max(w, h);
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
	Data<int> m_cellSize, m_threshold;
	Data<std::vector<Mesh>> m_meshes;
};

int GeneratorMesh_MarchingSquaresClass = RegisterObject<GeneratorMesh_MarchingSquares>("Generator/Mesh/Marching squares")
		.setDescription("Create a mesh by extracting a region of an image");

//****************************************************************************//


} // namespace Panda
