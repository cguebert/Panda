#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/ImageWrapper.h>

#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>

namespace panda {

using types::ImageWrapper;

class ShaderEffects : public PandaObject
{
public:
	PANDA_CLASS(ShaderEffects, PandaObject)

	ShaderEffects(PandaDocument* doc, int nbPasses)
		: PandaObject(doc)
		, m_nbPasses(nbPasses)
		, m_input(initData(&m_input, "input", "The original image"))
		, m_output(initData(&m_output, "output", "Image created by the operation"))
	{
		addInput(&m_input);
		addOutput(&m_output);

		m_texCoords[0*2+0] = 1; m_texCoords[0*2+1] = 1;
		m_texCoords[1*2+0] = 0; m_texCoords[1*2+1] = 1;
		m_texCoords[3*2+0] = 0; m_texCoords[3*2+1] = 0;
		m_texCoords[2*2+0] = 1; m_texCoords[2*2+1] = 0;
	}

	void postCreate()
	{
		PandaObject::postCreate();

		for(int i=0; i<m_nbPasses; ++i)
		{
			auto program = QSharedPointer<QOpenGLShaderProgram>(new QOpenGLShaderProgram());
			initShaderProgram(i, *program.data());
			m_shaderPrograms.push_back(program);
		}
	}

	/// Called by the constructor (load here the shaders)
	virtual void initShaderProgram(int passId, QOpenGLShaderProgram& program) = 0;

	/// Before we do any pass (get the Data values)
	virtual void prepareUpdate(QSize size) = 0;

	/// Called when doing a new pass
	virtual void preparePass(int passId, QOpenGLShaderProgram& program) = 0;

	void update()
	{
		const auto& inputVal = m_input.getValue();
		GLuint inputTexId = inputVal.getTextureId();
		if(inputTexId)
		{
			QSize inputSize = inputVal.size();
			auto outputAcc = m_output.getAccessor();
			QOpenGLFramebufferObject* outputFbo = outputAcc->getFbo();
			QOpenGLFramebufferObject* intermediaryFbo = m_intermediaryFbo.data();
			if(!outputFbo || outputFbo->size() != inputSize)
			{
				auto newFbo = QSharedPointer<QOpenGLFramebufferObject>(new QOpenGLFramebufferObject(inputSize));
				outputAcc->setFbo(newFbo);
				outputFbo = newFbo.data();

				if(m_nbPasses > 1)
				{
					m_intermediaryFbo.reset(new QOpenGLFramebufferObject(inputSize));
					intermediaryFbo = m_intermediaryFbo.data();
				}
			}

			glClearColor(0, 0, 0, 0);
			glViewport(0, 0, inputSize.width(), inputSize.height());

			QMatrix4x4 mvp = QMatrix4x4();
			mvp.ortho(0, inputSize.width(), inputSize.height(), 0, -10, 10);

			GLfloat verts[8];
			verts[0*2+0] = inputSize.width();	verts[0*2+1] = 0;
			verts[1*2+0] = 0;					verts[1*2+1] = 0;
			verts[3*2+0] = 0;					verts[3*2+1] = inputSize.height();
			verts[2*2+0] = inputSize.width();	verts[2*2+1] = inputSize.height();

			prepareUpdate(inputSize);

			for(int i=0; i<m_nbPasses; ++i)
			{
				QOpenGLFramebufferObject* destFbo = nullptr;
				GLuint texId = 0;

				if(m_nbPasses % 2) // odd # of passes, go first to output, then ping-pong
				{
					if(!i)			{ texId = inputTexId;					destFbo = outputFbo; }
					else if(i % 2)	{ texId = outputFbo->texture();			destFbo = intermediaryFbo; }
					else			{ texId = intermediaryFbo->texture();	destFbo = outputFbo; }
				}
				else // even: go to mid, then ping-pong
				{
					if(!i)			{ texId = inputTexId;					destFbo = intermediaryFbo; }
					else if(i % 2)	{ texId = intermediaryFbo->texture();	destFbo = outputFbo; }
					else			{ texId = outputFbo->texture();			destFbo = intermediaryFbo; }
				}

				auto& program = *m_shaderPrograms[i].data();
				program.bind();
				destFbo->bind();

				preparePass(i, program);

				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				glBindTexture(GL_TEXTURE_2D, texId);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D ,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

				program.setUniformValue("MVP", mvp);
				program.setUniformValue("tex0", 0);

				program.enableAttributeArray("vertex");
				program.setAttributeArray("vertex", verts, 2);

				program.enableAttributeArray("texCoord");
				program.setAttributeArray("texCoord", m_texCoords, 2);

				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

				program.disableAttributeArray("vertex");
				program.disableAttributeArray("texCoord");
				program.release();

				destFbo->release();
			}
		}
	}

protected:
	const int m_nbPasses;
	Data< ImageWrapper > m_input, m_output;

	QSharedPointer<QOpenGLFramebufferObject> m_intermediaryFbo;
	QVector< QSharedPointer<QOpenGLShaderProgram> > m_shaderPrograms;
	GLfloat m_texCoords[8];
};

//****************************************************************************//

class ModifierImage_GaussianBlur : public ShaderEffects
{
public:
	PANDA_CLASS(ModifierImage_GaussianBlur, ShaderEffects)

	ModifierImage_GaussianBlur(PandaDocument* doc)
		: ShaderEffects(doc, 2)
		, m_radius(initData(&m_radius, (PReal)10, "radius", "Radius of the blur"))
	{
		addInput(&m_radius);
	}

	void initShaderProgram(int passId, QOpenGLShaderProgram& program)
	{
		if(!passId)
		{
			program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/effects/GBlurH.v.glsl");
			program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/effects/GBlur.f.glsl");
			program.link();
		}
		else
		{
			program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/effects/GBlurV.v.glsl");
			program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/effects/GBlur.f.glsl");
			program.link();
		}
	}

	void prepareUpdate(QSize size)
	{
		PReal radius = m_radius.getValue() / 7;
		m_radiusScaleW = radius / size.width();
		m_radiusScaleW = radius / size.height();
	}

	void preparePass(int passId, QOpenGLShaderProgram& program)
	{
		if(!passId)
			program.setUniformValue("radiusScale", m_radiusScaleW);
		else
			program.setUniformValue("radiusScale", m_radiusScaleW);
	}

protected:
	Data< PReal > m_radius;
	PReal m_radiusScaleW, m_radiusScaleH;
};

int ModifierImage_GaussianBlurClass = RegisterObject<ModifierImage_GaussianBlur>("Modifier/Image/Effects/Gaussian blur")
		.setDescription("Apply a gaussian blur to an image");

//****************************************************************************//
/*
class ModifierImage_Enblend : public PandaObject
{
public:
	PANDA_CLASS(ModifierImage_Enblend, PandaObject)

	ModifierImage_Enblend(PandaDocument* doc)
		: PandaObject(doc)
		, m_input(initData(&m_input, "input", "The original image"))
		, m_output(initData(&m_output, "output", "Image created by the operation"))
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
		if(inputTexId)
		{
			QSize inputSize = inputVal.size();
			auto outputAcc = m_output.getAccessor();
			QOpenGLFramebufferObject* outputFbo = outputAcc->getFbo();
			if(!outputFbo || outputFbo->size() != inputSize)
			{
				auto newFbo = QSharedPointer<QOpenGLFramebufferObject>(new QOpenGLFramebufferObject(inputSize));
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
					m_laplacianFbos.push_back(QSharedPointer<QOpenGLFramebufferObject>(new QOpenGLFramebufferObject(size)));
					size /= 2;
					m_gaussianFbos.push_back(QSharedPointer<QOpenGLFramebufferObject>(new QOpenGLFramebufferObject(size)));
				}
			}
		}
	}

protected:
	Data< ImageWrapper > m_input, m_output;
	Data< int > m_levels;

	QVector< QSharedPointer<QOpenGLFramebufferObject> > m_gaussianFbos, m_laplacianFbos;
};

int ModifierImage_EnblendClass = RegisterObject<ModifierImage_Enblend>("Modifier/Image/Effects/Enblend")
		.setDescription("Tests of algorithms");
*/
} // namespace Panda
