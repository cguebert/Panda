#include <GL/glew.h>

#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Renderer.h>
#include <panda/types/Color.h>
#include <panda/types/ImageWrapper.h>
#include <panda/types/Rect.h>
#include <panda/types/Shader.h>
#include <panda/graphics/ShaderProgram.h>

namespace panda {

using types::Color;
using types::ImageWrapper;
using types::Rect;
using types::Shader;

class RenderRect : public Renderer
{
public:
	PANDA_CLASS(RenderRect, Renderer)

	RenderRect(PandaDocument* parent)
		: Renderer(parent)
		, rect(initData("rectangle", "Position and size of the rectangle"))
		, lineWidth(initData("lineWidth", "Width of the line"))
		, color(initData("color", "Color of the rectangle"))
		, shader(initData("shader", "Shaders used during the rendering"))
	{
		addInput(rect);
		addInput(lineWidth);
		addInput(color);
		addInput(shader);

		rect.getAccessor().push_back(Rect(100, 100, 150, 150));
		color.getAccessor().push_back(Color::black());
		lineWidth.getAccessor().push_back(0.0);

		shader.setWidgetData("Vertex;Fragment");
		auto shaderAcc = shader.getAccessor();
		shaderAcc->setSourceFromFile(Shader::ShaderType::Vertex, "shaders/PT_uniColor_noTex.v.glsl");
		shaderAcc->setSourceFromFile(Shader::ShaderType::Fragment, "shaders/PT_uniColor_noTex.f.glsl");
	}

	void render()
	{
		const std::vector<Rect>& listRect = rect.getValue();
		const std::vector<Color>& listColor = color.getValue();
		const std::vector<PReal>& listWidth = lineWidth.getValue();

		int nbRect = listRect.size();
		int nbColor = listColor.size();
		int nbWidth = listWidth.size();

		if(nbRect && nbColor || nbWidth)
		{
			if(!shader.getValue().apply(shaderProgram))
				return;

			if(nbColor < nbRect) nbColor = 1;
			if(nbWidth < nbRect) nbWidth = 1;
			PReal verts[8];

			shaderProgram.bind();
			shaderProgram.setUniformValueMat4("MVP", getMVPMatrix().constData());

			shaderProgram.enableAttributeArray("vertex");
			shaderProgram.setAttributeArray("vertex", verts, 2);

			int colorLocation = shaderProgram.uniformLocation("color");

			for(int i=0; i<nbRect; ++i)
			{
				shaderProgram.setUniformValueArray(colorLocation, listColor[i % nbColor].data(), 1, 4);

				glLineWidth(listWidth[i % nbWidth]);

				Rect rect = listRect[i % nbRect];
				verts[0*2+0] = rect.right(); verts[0*2+1] = rect.top();
				verts[1*2+0] = rect.left(); verts[1*2+1] = rect.top();
				verts[2*2+0] = rect.left(); verts[2*2+1] = rect.bottom();
				verts[3*2+0] = rect.right(); verts[3*2+1] = rect.bottom();

				glDrawArrays(GL_LINE_LOOP, 0, 4);
			}

			shaderProgram.disableAttributeArray("vertex");
			shaderProgram.release();
		}
	}

protected:
	Data< std::vector<Rect> > rect;
	Data< std::vector<PReal> > lineWidth;
	Data< std::vector<Color> > color;
	Data< Shader > shader;

	graphics::ShaderProgram shaderProgram;
};

int RenderRectClass = RegisterObject<RenderRect>("Render/Line/Rectangle")
		.setName("Rectangle").setDescription("Draw a rectangle");

//****************************************************************************//

class RenderFilledRect : public Renderer
{
public:
	PANDA_CLASS(RenderFilledRect, Renderer)

	RenderFilledRect(PandaDocument* parent)
		: Renderer(parent)
		, rect(initData("rectangle", "Position and size of the rectangle"))
		, color(initData("color", "Color of the rectangle"))
		, shader(initData("shader", "Shaders used during the rendering"))
	{
		addInput(rect);
		addInput(color);
		addInput(shader);

		rect.getAccessor().push_back(Rect(100, 100, 150, 150));
		color.getAccessor().push_back(Color::black());

		shader.setWidgetData("Vertex;Fragment");
		auto shaderAcc = shader.getAccessor();
		shaderAcc->setSourceFromFile(Shader::ShaderType::Vertex, "shaders/PT_uniColor_noTex.v.glsl");
		shaderAcc->setSourceFromFile(Shader::ShaderType::Fragment, "shaders/PT_uniColor_noTex.f.glsl");

		m_texCoords[0*2+0] = 1; m_texCoords[0*2+1] = 1;
		m_texCoords[1*2+0] = 0; m_texCoords[1*2+1] = 1;
		m_texCoords[3*2+0] = 0; m_texCoords[3*2+1] = 0;
		m_texCoords[2*2+0] = 1; m_texCoords[2*2+1] = 0;
	}

	void render()
	{
		const std::vector<Rect>& listRect = rect.getValue();
		const std::vector<Color>& listColor = color.getValue();

		int nbRect = listRect.size();
		int nbColor = listColor.size();

		if(nbRect && nbColor)
		{
			if(!shader.getValue().apply(shaderProgram))
				return;

			if(nbColor < nbRect) nbColor = 1;
			PReal verts[8];

			shaderProgram.setUniformValueMat4("MVP", getMVPMatrix().constData());

			shaderProgram.enableAttributeArray("vertex");
			shaderProgram.setAttributeArray("vertex", verts, 2);

			int colorLocation = shaderProgram.uniformLocation("color");

			int texCoordLocation = shaderProgram.attributeLocation("texCoord");
			if(texCoordLocation != -1)
			{
				shaderProgram.enableAttributeArray(texCoordLocation);
				shaderProgram.setAttributeArray(texCoordLocation, m_texCoords, 2);
			}

			for(int i=0; i<nbRect; ++i)
			{
				shaderProgram.setUniformValueArray(colorLocation, listColor[i % nbColor].data(), 1, 4);

				Rect rect = listRect[i % nbRect];
				verts[0*2+0] = rect.right(); verts[0*2+1] = rect.top();
				verts[1*2+0] = rect.left(); verts[1*2+1] = rect.top();
				verts[2*2+0] = rect.right(); verts[2*2+1] = rect.bottom();
				verts[3*2+0] = rect.left(); verts[3*2+1] = rect.bottom();

				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			}

			shaderProgram.disableAttributeArray("vertex");
			if(texCoordLocation != -1)
				shaderProgram.disableAttributeArray(texCoordLocation);
			shaderProgram.release();
		}
	}

protected:
	Data< std::vector<Rect> > rect;
	Data< std::vector<Color> > color;
	Data< Shader > shader;

	GLfloat m_texCoords[8];

	graphics::ShaderProgram shaderProgram;
};

int RenderFilledRectClass = RegisterObject<RenderFilledRect>("Render/Filled/Rectangle")
		.setName("Filled rectangle").setDescription("Draw a filled rectangle");

//****************************************************************************//

class RenderTexturedRect : public Renderer
{
public:
	PANDA_CLASS(RenderTexturedRect, Renderer)

	RenderTexturedRect(PandaDocument* parent)
		: Renderer(parent)
		, rect(initData("rectangle", "Position and size of the rectangle"))
		, texture(initData("texture", "Texture to apply to the rectangle"))
		, shader(initData("shader", "Shaders used during the rendering"))
	{
		addInput(rect);
		addInput(texture);
		addInput(shader);

		rect.getAccessor().push_back(Rect(100, 100, 150, 150));

		shader.setWidgetData("Vertex;Fragment");
		auto shaderAcc = shader.getAccessor();
		shaderAcc->setSourceFromFile(Shader::ShaderType::Vertex, "shaders/PT_noColor_Tex.v.glsl");
		shaderAcc->setSourceFromFile(Shader::ShaderType::Fragment, "shaders/PT_noColor_Tex.f.glsl");

		m_texCoords[0*2+0] = 1; m_texCoords[0*2+1] = 1;
		m_texCoords[1*2+0] = 0; m_texCoords[1*2+1] = 1;
		m_texCoords[3*2+0] = 0; m_texCoords[3*2+1] = 0;
		m_texCoords[2*2+0] = 1; m_texCoords[2*2+1] = 0;
	}

	void render()
	{
		const std::vector<Rect>& listRect = rect.getValue();

		int nbRect = listRect.size();
		int texId = texture.getValue().getTextureId();

		if(nbRect && texId)
		{
			if(!shader.getValue().apply(shaderProgram))
				return;

			PReal verts[8];

			shaderProgram.setUniformValueMat4("MVP", getMVPMatrix().constData());

			shaderProgram.enableAttributeArray("vertex");
			shaderProgram.setAttributeArray("vertex", verts, 2);

			shaderProgram.enableAttributeArray("texCoord");
			shaderProgram.setAttributeArray("texCoord", m_texCoords, 2);

			glBindTexture(GL_TEXTURE_2D, texId);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D ,GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			shaderProgram.setUniformValue("tex0", 0);

			for(int i=0; i<nbRect; ++i)
			{
				Rect rect = listRect[i % nbRect];
				verts[0*2+0] = rect.right(); verts[0*2+1] = rect.top();
				verts[1*2+0] = rect.left(); verts[1*2+1] = rect.top();
				verts[2*2+0] = rect.right(); verts[2*2+1] = rect.bottom();
				verts[3*2+0] = rect.left(); verts[3*2+1] = rect.bottom();

				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			}

			shaderProgram.disableAttributeArray("vertex");
			shaderProgram.disableAttributeArray("texCoord");
			shaderProgram.release();
		}
	}

protected:
	Data< std::vector<Rect> > rect;
	Data< ImageWrapper > texture;
	Data< Shader > shader;

	GLfloat m_texCoords[8];

	graphics::ShaderProgram shaderProgram;
};

int RenderTexturedRectClass = RegisterObject<RenderTexturedRect>("Render/Textured/Rectangle")
		.setName("Textured rectangle").setDescription("Draw a textured rectangle");


} // namespace panda
