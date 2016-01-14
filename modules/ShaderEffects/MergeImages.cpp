#include <panda/PandaDocument.h>
#include <panda/object/ObjectFactory.h>
#include <panda/object/OGLObject.h>
#include <panda/types/ImageWrapper.h>
#include <panda/graphics/Framebuffer.h>
#include <panda/graphics/ShaderProgram.h>
#include <panda/helper/system/FileRepository.h>

#include <GL/glew.h>

#include <QMatrix4x4>

namespace panda {

using types::ImageWrapper;

class ModifierImage_MergeImages : public OGLObject
{
public:
	PANDA_CLASS(ModifierImage_MergeImages, OGLObject)

	ModifierImage_MergeImages(PandaDocument *doc)
		: OGLObject(doc)
		, m_input1(initData("input1", "The first input image (in front)"))
		, m_input2(initData("input2", "The second input image (in back)"))
		, m_output(initData("output", "The combined image"))
		, m_compositionMode(initData(0, "composition mode", "Defines how the 2 images are merged"))
	{
		addInput(m_input1);
		addInput(m_input2);
		addInput(m_compositionMode);

		addOutput(m_output);

		// 24 possible modes
		m_compositionMode.setWidget("enum");
		m_compositionMode.setWidgetData("SourceOver;DestinationOver;Clear;Source;Destination;"
									  "SourceIn;DestinationIn;SourceOut;DestinationOut;SourceAtop;DestinationAtop;"
									  "Xor;Plus;Multiply;Screen;Overlay;Darken;Lighten;"
									  "ColorDodge;ColorBurn;HardLight;SoftLight;Difference;Exclusion");

		m_texCoords[0*2+0] = 1; m_texCoords[0*2+1] = 1;
		m_texCoords[1*2+0] = 0; m_texCoords[1*2+1] = 1;
		m_texCoords[3*2+0] = 0; m_texCoords[3*2+1] = 0;
		m_texCoords[2*2+0] = 1; m_texCoords[2*2+1] = 0;
	}

	void initializeGL()
	{
		m_shaderProgram.addShaderFromMemory(graphics::ShaderType::Vertex,
			helper::system::DataRepository.loadFile("shaders/mergeLayers.v.glsl"));
		m_shaderProgram.addShaderFromMemory(graphics::ShaderType::Fragment,
			helper::system::DataRepository.loadFile("shaders/mergeLayers.f.glsl"));
		m_shaderProgram.link();
	}

	void update()
	{
		const auto& input1Val = m_input1.getValue();
		const auto& input2Val = m_input2.getValue();
		GLuint tex1Id = input1Val.getTextureId();
		GLuint tex2Id = input2Val.getTextureId();
		if(tex1Id && tex2Id && input1Val.size() == input2Val.size())
		{
			auto inputSize = input1Val.size();
			auto outputAcc = m_output.getAccessor();
			auto outputFbo = outputAcc->getFbo();
			if(!outputFbo || outputFbo->size() != inputSize)
			{
				outputAcc->setFbo(graphics::Framebuffer(inputSize));
				outputFbo = outputAcc->getFbo();
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

			m_shaderProgram.bind();
			outputFbo->bind();
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tex1Id);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, tex2Id);
			glActiveTexture(GL_TEXTURE0);

			m_shaderProgram.setUniformValueMat4("MVP", mvp.constData());
			m_shaderProgram.setUniformValue("texS", 0);
			m_shaderProgram.setUniformValue("texD", 1);

			m_shaderProgram.enableAttributeArray("vertex");
			m_shaderProgram.setAttributeArray("vertex", verts, 2);

			m_shaderProgram.enableAttributeArray("texCoord");
			m_shaderProgram.setAttributeArray("texCoord", m_texCoords, 2);

			m_shaderProgram.setUniformValue("mode", m_compositionMode.getValue());
			m_shaderProgram.setUniformValue("opacity", 1.0f);

			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			m_shaderProgram.disableAttributeArray("vertex");
			m_shaderProgram.disableAttributeArray("texCoord");
			m_shaderProgram.release();

			outputFbo->release();
		}

		cleanDirty();
	}

protected:
	Data< ImageWrapper > m_input1, m_input2, m_output;
	Data< int > m_compositionMode;

	graphics::ShaderProgram m_shaderProgram;
	GLfloat m_texCoords[8];
};

int ModifierImage_MergeImagesClass = RegisterObject<ModifierImage_MergeImages>("Modifier/Image/Merge images")
		.setDescription("Combine 2 images (same operation as with layers)");

} // namespace Panda
