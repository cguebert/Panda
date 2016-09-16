#include <panda/document/RenderedDocument.h>
#include <panda/object/ObjectFactory.h>
#include <panda/object/OGLObject.h>
#include <panda/types/ImageWrapper.h>
#include <panda/graphics/Framebuffer.h>
#include <panda/graphics/Mat4x4.h>
#include <panda/graphics/Buffer.h>
#include <panda/graphics/ShaderProgram.h>
#include <panda/graphics/VertexArrayObject.h>
#include <panda/helper/ShaderCache.h>

#include <GL/glew.h>

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
	}

	void initializeGL()
	{
		using ShaderType = panda::graphics::ShaderType;
		m_shaderProgram = panda::helper::ShaderCache::getInstance()->getShaderProgramFromFile({
			{ ShaderType::Vertex, "shaders/mergeLayers.v.glsl" },
			{ ShaderType::Fragment, "shaders/mergeLayers.f.glsl" }
		});

		m_VAO.create();
		m_VAO.bind();

		m_verticesVBO.create();
		m_verticesVBO.bind();
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
		glEnableVertexAttribArray(0);

		m_texCoordsVBO.create();
		m_texCoordsVBO.bind();
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
		glEnableVertexAttribArray(1);

		std::vector<GLfloat> texCoords = { 0, 1, 1, 1, 0, 0, 1, 0 };
		m_texCoordsVBO.write(texCoords);

		m_VAO.release();
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
			GLfloat w = static_cast<GLfloat>(inputSize.width()), h = static_cast<GLfloat>(inputSize.height());
			if(!outputFbo || outputFbo->size() != inputSize)
			{
				outputAcc->setFbo(graphics::Framebuffer(inputSize));
				outputFbo = outputAcc->getFbo();

				std::vector<GLfloat> verts = { 0, 0, w, 0, 0, h, w, h };
				m_verticesVBO.bind();
				m_verticesVBO.write(verts);
			}

			m_shaderProgram.bind();
			outputFbo->bind();

			glViewport(0, 0, inputSize.width(), inputSize.height());
			glClearColor(0, 0, 0, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tex1Id);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, tex2Id);
			glActiveTexture(GL_TEXTURE0);

			graphics::Mat4x4 mvp;
			mvp.ortho(0, w, h, 0, -10, 10);
			m_shaderProgram.setUniformValueMat4("MVP", mvp.data());
			m_shaderProgram.setUniformValue("texS", 0);
			m_shaderProgram.setUniformValue("texD", 1);

			m_shaderProgram.setUniformValue("mode", m_compositionMode.getValue());
			m_shaderProgram.setUniformValue("opacity", 1.0f);

			m_VAO.bind();
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			m_VAO.release();

			m_shaderProgram.release();
			outputFbo->release();
		}
	}

protected:
	Data< ImageWrapper > m_input1, m_input2, m_output;
	Data< int > m_compositionMode;

	graphics::ShaderProgram m_shaderProgram;
	graphics::VertexArrayObject m_VAO;
	graphics::Buffer m_verticesVBO, m_texCoordsVBO;
};

int ModifierImage_MergeImagesClass = RegisterObject<ModifierImage_MergeImages, RenderedDocument>("Modifier/Image/Merge images")
		.setDescription("Combine 2 images (same operation as with layers)");

} // namespace Panda
