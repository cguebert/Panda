#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/ImageWrapper.h>

#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>

namespace panda {

using types::ImageWrapper;

/*
class ModifierImage_Enblend : public PandaObject
{
public:
	PANDA_CLASS(ModifierImage_Enblend, PandaObject)

	ModifierImage_Enblend(PandaDocument* doc)
		: PandaObject(doc)
		, m_input(initData("input", "The original image"))
		, m_output(initData("output", "Image created by the operation"))
		, m_levels(initData(4, "levels", "Number of levels to use"))
	{
		addInput(m_input);
		addInput(m_levels);

		addOutput(m_output);
	}

	void update()
	{
		cleanDirty();

		const auto& inputVal = m_input.getValue();
		GLuint inputTexId = inputVal.getTextureId();
		if(inputTexId)
		{
			QSize inputSize = inputVal.size();
			auto outputAcc = m_output.getAccessor();
			QOpenGLFramebufferObject* outputFbo = outputAcc->getFbo();
			if(!outputFbo || outputFbo->size() != inputSize)
			{
				auto newFbo = std::make_shared<QOpenGLFramebufferObject>(inputSize);
				outputAcc->setFbo(newFbo);
				outputFbo = newFbo.data();

				m_fbos.clear();
			}

			int nbLevels = m_levels.getValue();
			if(nbLevels != m_fbos.size())
			{
				QSize size = inputSize;
				for(int i=0; i<nbLevels; ++i)
				{
					m_laplacianFbos.push_back(std::make_shared<QOpenGLFramebufferObject>(size));
					size /= 2;
					m_gaussianFbos.push_back(std::make_shared<QOpenGLFramebufferObject>(size));
				}
			}
		}
	}

protected:
	Data< ImageWrapper > m_input, m_output;
	Data< int > m_levels;

	std::vector< std::shared_ptr<QOpenGLFramebufferObject> > m_gaussianFbos, m_laplacianFbos;
};

int ModifierImage_EnblendClass = RegisterObject<ModifierImage_Enblend>("Modifier/Image/Effects/Enblend")
		.setDescription("Tests of algorithms");
*/
} // namespace Panda
