#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/ImageWrapper.h>

#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>

namespace panda {

using types::ImageWrapper;

class ModifierImage_GaussianPyramid : public PandaObject
{
public:
	PANDA_CLASS(ModifierImage_GaussianPyramid, PandaObject)

	ModifierImage_GaussianPyramid(PandaDocument* doc)
		: PandaObject(doc)
		, m_input(initData(&m_input, "input", "The original image"))
		, m_output(initData(&m_output, "output", "List of scaled down images"))
		, m_levels(initData(&m_levels, 4, "levels", "Number of levels to use"))
	{
		addInput(&m_input);
		addInput(&m_levels);

		addOutput(&m_output);
	}

	void update()
	{
		cleanDirty();

		const auto& inputVal = m_input.getValue();
		GLuint inputTexId = inputVal.getTextureId();
		int nbLevels = m_levels.getValue();
		if(inputTexId && nbLevels)
		{
			QSize inputSize = inputVal.size();
			auto outputAcc = m_output.getAccessor();
			if(outputAcc.size() != nbLevels || outputAcc[0].size() != inputSize)
			{
				outputAcc.resize(nbLevels);
				QSize size = inputSize;
				for(int i=0; i<nbLevels; ++i)
				{
					auto newFbo = QSharedPointer<QOpenGLFramebufferObject>(new QOpenGLFramebufferObject(size));
					outputAcc[i].setFbo(newFbo);
					size /= 2;
				}
			}
		}
		else
			m_output.getAccessor().clear();
	}

protected:
	Data< ImageWrapper > m_input;
	Data< QVector< ImageWrapper > > m_output;
	Data< int > m_levels;
};

int ModifierImage_GaussianPyramidClass = RegisterObject<ModifierImage_GaussianPyramid>("Modifier/Image/Gaussian pyramid")
		.setDescription("Create a stack of successively smaller images, each scaled down from the previous one");

} // namespace Panda
