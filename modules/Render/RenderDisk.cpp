#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Renderer.h>
#include <panda/types/Color.h>
#include <panda/types/ImageWrapper.h>
#include <panda/types/Point.h>
#include <panda/types/Gradient.h>
#include <panda/types/Shader.h>
#include <panda/helper/GradientCache.h>

#include <QOpenGLShaderProgram>
#include <cmath>

namespace panda {

using types::Color;
using types::ImageWrapper;
using types::Point;
using types::Gradient;
using types::Shader;
using helper::GradientCache;

class RenderDisk : public Renderer
{
public:
	PANDA_CLASS(RenderDisk, Renderer)

	RenderDisk(PandaDocument* parent)
		: Renderer(parent)
		, center(initData("center", "Center position of the disk"))
		, radius(initData("radius", "Radius of the disk"))
		, color(initData("color", "Color of the plain disk"))
		, shader(initData("shader", "Shaders used during the rendering"))
	{
		addInput(center);
		addInput(radius);
		addInput(color);
		addInput(shader);

		center.getAccessor().push_back(Point(100, 100));
		radius.getAccessor().push_back(5.0);
		color.getAccessor().push_back(Color::black());

		shader.setWidgetData("Vertex;Fragment");
		auto shaderAcc = shader.getAccessor();
		shaderAcc->setSourceFromFile(QOpenGLShader::Vertex, "shaders/PT_uniColor_noTex.v.glsl");
		shaderAcc->setSourceFromFile(QOpenGLShader::Fragment, "shaders/PT_uniColor_noTex.f.glsl");
	}

	void update()
	{
		const std::vector<Point>& listCenter = center.getValue();
		const std::vector<PReal>& listRadius = radius.getValue();
		const std::vector<Color>& listColor = color.getValue();

		int nbCenter = listCenter.size();
		int nbRadius = listRadius.size();
		int nbColor = listColor.size();

		vertexBuffer.clear();
		firstBuffer.clear();
		countBuffer.clear();
		colorBuffer.clear();

		if(nbCenter && nbRadius && nbColor)
		{
			if(nbRadius < nbCenter) nbRadius = 1;
			if(nbColor < nbCenter) nbColor = 1;

			PReal PI2 = static_cast<PReal>(M_PI) * 2;
			for(int i=0; i<nbCenter; ++i)
			{
				PReal valRadius = listRadius[i % nbRadius];
				int nbSeg = static_cast<int>(floor(valRadius * PI2));
				if(nbSeg < 3) continue;

				colorBuffer.push_back(listColor[i % nbColor]);

				int nbVertices = vertexBuffer.size();
				firstBuffer.push_back(nbVertices);
				countBuffer.push_back(nbSeg + 2);

				vertexBuffer.resize(nbVertices + nbSeg + 2);

				const Point& valCenter = listCenter[i];
				vertexBuffer[nbVertices] = valCenter;

				PReal angle = PI2 / nbSeg;
				PReal ca = cos(angle), sa = sin(angle);
				Point dir = Point(valRadius, 0);

				for(int i=0; i<=nbSeg; ++i)
				{
					Point pt = Point(dir.x*ca+dir.y*sa, dir.y*ca-dir.x*sa);
					vertexBuffer[nbVertices + 1 + i] = valCenter + pt;
					dir = pt;
				}
			}
		}

		cleanDirty();
	}

	void render()
	{
		if(vertexBuffer.empty())
			return;

		if(!shader.getValue().apply(shaderProgram))
			return;

		shaderProgram.bind();
		shaderProgram.setUniformValue("MVP", getMVPMatrix());

		shaderProgram.enableAttributeArray("vertex");
		shaderProgram.setAttributeArray("vertex", vertexBuffer.front().data(), 2);

		int colorLocation = shaderProgram.uniformLocation("color");

		int nb = countBuffer.size();
		for(int i=0; i<nb; ++i)
		{
			auto color = colorBuffer[i];
			shaderProgram.setUniformValue(colorLocation, color.r, color.g, color.b, color.a);

			glDrawArrays(GL_TRIANGLE_FAN, firstBuffer[i], countBuffer[i]);
		}

		shaderProgram.disableAttributeArray("vertex");
		shaderProgram.release();
	}

protected:
	Data< std::vector<Point> > center;
	Data< std::vector<PReal> > radius;
	Data< std::vector<Color> > color;
	Data< Shader > shader;

	std::vector<Point> vertexBuffer;
	std::vector<GLint> firstBuffer;
	std::vector<GLsizei> countBuffer;
	std::vector<Color> colorBuffer;

	QOpenGLShaderProgram shaderProgram;
};

int RenderDiskClass = RegisterObject<RenderDisk>("Render/Filled/Disk").setDescription("Draw a plain disk");

//****************************************************************************//

class RenderDisk_Gradient : public Renderer
{
public:
	PANDA_CLASS(RenderDisk_Gradient, Renderer)

	RenderDisk_Gradient(PandaDocument* parent)
		: Renderer(parent)
		, center(initData("center", "Center position of the disk"))
		, radius(initData("radius", "Radius of the disk"))
		, gradient(initData("gradient", "Gradient used to fill the disk"))
		, shader(initData("shader", "Shaders used during the rendering"))
	{
		addInput(center);
		addInput(radius);
		addInput(gradient);
		addInput(shader);

		center.getAccessor().push_back(Point(100, 100));
		radius.getAccessor().push_back(5.0);
		gradient.getAccessor().push_back(Gradient::defaultGradient());

		shader.setWidgetData("Vertex;Fragment");
		auto shaderAcc = shader.getAccessor();
		shaderAcc->setSourceFromFile(QOpenGLShader::Vertex, "shaders/PT_noColor_Tex.v.glsl");
		shaderAcc->setSourceFromFile(QOpenGLShader::Fragment, "shaders/PT_noColor_Tex.f.glsl");
	}

	void update()
	{
		const std::vector<Point>& listCenter = center.getValue();
		const std::vector<PReal>& listRadius = radius.getValue();

		int nbCenter = listCenter.size();
		int nbRadius = listRadius.size();

		vertexBuffer.clear();
		texCoordsBuffer.clear();
		firstBuffer.clear();
		countBuffer.clear();

		if(nbCenter && nbRadius)
		{
			if(nbRadius < nbCenter) nbRadius = 1;

			PReal PI2 = static_cast<PReal>(M_PI) * 2;
			for(int i=0; i<nbCenter; ++i)
			{
				PReal valRadius = listRadius[i % nbRadius];
				int nbSeg = static_cast<int>(floor(valRadius * PI2));
				if(nbSeg < 3)
				{
					firstBuffer.push_back(0);
					countBuffer.push_back(0);
					continue;
				}

				int nbVertices = vertexBuffer.size();
				firstBuffer.push_back(nbVertices);
				countBuffer.push_back(nbSeg + 2);

				vertexBuffer.resize(nbVertices + nbSeg + 2);
				texCoordsBuffer.resize(nbVertices + nbSeg + 2);

				const Point& valCenter = listCenter[i];
				vertexBuffer[nbVertices] = valCenter;
				texCoordsBuffer[nbVertices] = Point(0, 0);

				PReal angle = PI2 / nbSeg;
				PReal ca = cos(angle), sa = sin(angle);
				Point dir = Point(valRadius, 0);
				PReal step = 1.0 / nbSeg;
				PReal texY = 0;

				for(int i=0; i<=nbSeg; ++i)
				{
					Point pt = Point(dir.x*ca+dir.y*sa, dir.y*ca-dir.x*sa);
					vertexBuffer[nbVertices + 1 + i] = valCenter + pt;
					texCoordsBuffer[nbVertices + 1 + i] = Point(1, texY);
					dir = pt;
					texY += step;
				}
			}
		}

		cleanDirty();
	}

	void render()
	{
		const std::vector<PReal>& listRadius = radius.getValue();
		const std::vector<Gradient>& listGradient = gradient.getValue();

		int nbRadius = listRadius.size();
		int nbGradient = listGradient.size();
		int nbDisks = countBuffer.size();

		if(nbDisks && nbGradient)
		{
			if(nbRadius < nbDisks) nbRadius = 1;
			if(nbGradient < nbDisks) nbGradient = 1;

			if(!shader.getValue().apply(shaderProgram))
				return;

			shaderProgram.bind();
			shaderProgram.setUniformValue("MVP", getMVPMatrix());

			shaderProgram.enableAttributeArray("vertex");
			shaderProgram.setAttributeArray("vertex", vertexBuffer.front().data(), 2);

			shaderProgram.enableAttributeArray("texCoord");
			shaderProgram.setAttributeArray("texCoord", texCoordsBuffer.front().data(), 2);

			shaderProgram.setUniformValue("tex0", 0);

			// Optimization when we use only one gradient
			if(nbGradient == 1)
			{
				PReal maxRadius = *std::max_element(listRadius.begin(), listRadius.end());
				GLuint texture = GradientCache::getInstance()->getTexture(listGradient.front(), static_cast<int>(ceil(maxRadius)));

				glBindTexture(GL_TEXTURE_2D, texture);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D ,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

				for(int i=0; i<nbDisks; ++i)
					glDrawArrays(GL_TRIANGLE_FAN, firstBuffer[i], countBuffer[i]);
			}
			else
			{

				for(int i=0; i<nbDisks; ++i)
				{
					GLuint texture = GradientCache::getInstance()->getTexture(listGradient[i % nbGradient], static_cast<int>(ceil(listRadius[i % nbRadius])));
					if(!texture)
						continue;
					glBindTexture(GL_TEXTURE_2D, texture);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D ,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

					glDrawArrays(GL_TRIANGLE_FAN, firstBuffer[i], countBuffer[i]);
			}
			}

			shaderProgram.disableAttributeArray("vertex");
			shaderProgram.disableAttributeArray("texCoord");
			shaderProgram.release();
		}
	}

protected:
	Data< std::vector<Point> > center;
	Data< std::vector<PReal> > radius;
	Data< std::vector<Gradient> > gradient;
	Data< Shader > shader;

	std::vector<Point> vertexBuffer, texCoordsBuffer;
	std::vector<GLint> firstBuffer;
	std::vector<GLsizei> countBuffer;

	QOpenGLShaderProgram shaderProgram;
};

int RenderDisk_GradientClass = RegisterObject<RenderDisk_Gradient>("Render/Gradient/Disk")
		.setName("Gradient disk").setDescription("Draw a disk filled with a radial gradient");

//****************************************************************************//

class RenderDisk_Textured : public Renderer
{
public:
	PANDA_CLASS(RenderDisk_Textured, Renderer)

	RenderDisk_Textured(PandaDocument* parent)
		: Renderer(parent)
		, center(initData("center", "Center position of the disk"))
		, radius(initData("radius", "Radius of the disk"))
		, texture(initData("texture", "Texture applied to the disk"))
		, shader(initData("shader", "Shaders used during the rendering"))
	{
		addInput(center);
		addInput(radius);
		addInput(texture);
		addInput(shader);

		center.getAccessor().push_back(Point(100, 100));
		radius.getAccessor().push_back(5.0);

		shader.setWidgetData("Vertex;Fragment");
		auto shaderAcc = shader.getAccessor();
		shaderAcc->setSourceFromFile(QOpenGLShader::Vertex, "shaders/PT_noColor_Tex.v.glsl");
		shaderAcc->setSourceFromFile(QOpenGLShader::Fragment, "shaders/PT_noColor_Tex.f.glsl");
	}

	void update()
	{
		const std::vector<Point>& listCenter = center.getValue();
		const std::vector<PReal>& listRadius = radius.getValue();

		int nbCenter = listCenter.size();
		int nbRadius = listRadius.size();

		vertexBuffer.clear();
		texCoordsBuffer.clear();
		firstBuffer.clear();
		countBuffer.clear();

		if(nbCenter && nbRadius)
		{
			if(nbRadius < nbCenter) nbRadius = 1;

			PReal PI2 = static_cast<PReal>(M_PI) * 2;
			for(int i=0; i<nbCenter; ++i)
			{
				PReal valRadius = listRadius[i % nbRadius];
				int nbSeg = static_cast<int>(floor(valRadius * PI2));
				if(nbSeg < 3)
				{
					firstBuffer.push_back(0);
					countBuffer.push_back(0);
					continue;
				}

				int nbVertices = vertexBuffer.size();
				firstBuffer.push_back(nbVertices);
				countBuffer.push_back(nbSeg + 2);

				vertexBuffer.resize(nbVertices + nbSeg + 2);
				texCoordsBuffer.resize(nbVertices + nbSeg + 2);

				const Point& valCenter = listCenter[i];
				vertexBuffer[nbVertices] = valCenter;
				texCoordsBuffer[nbVertices] = Point(0.5, 0.5);

				PReal angle = PI2 / nbSeg;
				PReal ca = cos(angle), sa = sin(angle);
				Point dir = Point(1, 0);
				Point texCenter = Point(0.5, 0.5);
				Point texScale = Point(0.5, -0.5);

				for(int i=0; i<=nbSeg; ++i)
				{
					Point pt = Point(dir.x*ca+dir.y*sa, dir.y*ca-dir.x*sa);
					vertexBuffer[nbVertices + 1 + i] = valCenter + pt * valRadius;
					texCoordsBuffer[nbVertices + 1 + i] = texCenter + pt.linearProduct(texScale);
					dir = pt;
				}
			}
		}

		cleanDirty();
	}

	void render()
	{
		const std::vector<PReal>& listRadius = radius.getValue();
		const int texId = texture.getValue().getTextureId();

		int nbRadius = listRadius.size();
		int nbDisks = countBuffer.size();

		if(nbDisks && texId)
		{
			if(nbRadius < nbDisks) nbRadius = 1;

			if(!shader.getValue().apply(shaderProgram))
				return;

			shaderProgram.bind();
			shaderProgram.setUniformValue("MVP", getMVPMatrix());

			shaderProgram.enableAttributeArray("vertex");
			shaderProgram.setAttributeArray("vertex", vertexBuffer.front().data(), 2);

			shaderProgram.enableAttributeArray("texCoord");
			shaderProgram.setAttributeArray("texCoord", texCoordsBuffer.front().data(), 2);

			glBindTexture(GL_TEXTURE_2D, texId);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D ,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			shaderProgram.setUniformValue("tex0", 0);

			for(int i=0; i<nbDisks; ++i)
				glDrawArrays(GL_TRIANGLE_FAN, firstBuffer[i], countBuffer[i]);

			shaderProgram.disableAttributeArray("vertex");
			shaderProgram.disableAttributeArray("texCoord");
			shaderProgram.release();
		}
	}

protected:
	Data< std::vector<Point> > center;
	Data< std::vector<PReal> > radius;
	Data< ImageWrapper > texture;
	Data< Shader > shader;

	std::vector<Point> vertexBuffer, texCoordsBuffer;
	std::vector<GLint> firstBuffer;
	std::vector<GLsizei> countBuffer;

	QOpenGLShaderProgram shaderProgram;
};

int RenderDisk_TexturedClass = RegisterObject<RenderDisk_Textured>("Render/Textured/Disk")
		.setName("Textured disk").setDescription("Draw a textured disk");

} // namespace panda
