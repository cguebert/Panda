#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Renderer.h>
#include <panda/types/Color.h>
#include <panda/types/Gradient.h>
#include <panda/types/Path.h>
#include <panda/types/Shader.h>
#include <panda/helper/GradientCache.h>
#include <panda/graphics/ShaderProgram.h>

namespace panda {

using types::Color;
using types::Gradient;
using types::Point;
using types::Path;
using types::Shader;
using helper::GradientCache;

class RenderLine : public Renderer
{
public:
	PANDA_CLASS(RenderLine, Renderer)

	RenderLine(PandaDocument* parent)
		: Renderer(parent)
		, inputA(initData("point 1", "Start of the line"))
		, inputB(initData("point 2", "End of the line"))
		, width(initData((PReal)1.0, "width", "Width of the line" ))
		, color(initData("color", "Color of the line"))
		, shader(initData("shader", "Shaders used during the rendering"))
	{
		addInput(inputA);
		addInput(inputB);
		addInput(width);
		addInput(color);
		addInput(shader);

		color.getAccessor().push_back(Color::black());

		shader.setWidgetData("Vertex;Fragment");
		auto shaderAcc = shader.getAccessor();
		shaderAcc->setSourceFromFile(Shader::ShaderType::Vertex, "shaders/PT_attColor_noTex.v.glsl");
		shaderAcc->setSourceFromFile(Shader::ShaderType::Fragment, "shaders/PT_attColor_noTex.f.glsl");
	}

	void render()
	{
		const std::vector<Point>& valA = inputA.getValue();
		const std::vector<Point>& valB = inputB.getValue();
		const std::vector<Color>& listColor = color.getValue();

		int nbA = valA.size(), nbB = valB.size();
		int nbLines = qMin(valA.size(), valB.size());
		bool useTwoLists = true;
		if(nbA && !nbB)
		{
			useTwoLists = false;
			nbLines = nbA / 2;
		}

		int nbColor = listColor.size();

		if(nbLines && nbColor)
		{
			if(!shader.getValue().apply(shaderProgram))
				return;

			if(nbColor < nbLines) nbColor = 1;
			int nbVertices = nbLines * 2;

			shaderProgram.bind();
			shaderProgram.setUniformValueMat4("MVP", getMVPMatrix().constData());

			std::vector<Point> tmpVertices;
			int vertexLocation = shaderProgram.attributeLocation("vertex");
			shaderProgram.enableAttributeArray(vertexLocation);
			if(useTwoLists)
			{
				tmpVertices.resize(nbVertices);
				for(int i=0; i<nbLines; ++i)
				{
					tmpVertices[i*2  ] = valA[i];
					tmpVertices[i*2+1] = valB[i];
				}
				shaderProgram.setAttributeArray(vertexLocation, tmpVertices.front().data(), 2);
			}
			else
				shaderProgram.setAttributeArray(vertexLocation, valA.front().data(), 2);

			std::vector<Color> tmpColors;
			int colorLocation = shaderProgram.attributeLocation("color");
			shaderProgram.enableAttributeArray(colorLocation);
			if(nbColor >= nbVertices) // We have one color for each vertex
				shaderProgram.setAttributeArray(colorLocation, listColor.front().data(), 4);
			else
			{
				tmpColors.resize(nbVertices);
				for(int i=0; i<nbLines; ++i)
					tmpColors[i*2] = tmpColors[i*2+1] = listColor[i%nbColor];
				shaderProgram.setAttributeArray(colorLocation, tmpColors.front().data(), 4);
			}

			glLineWidth(qMax((PReal)1.0, width.getValue()));
			glDrawArrays(GL_LINES, 0, nbVertices);
			glLineWidth(0);

			shaderProgram.disableAttributeArray(vertexLocation);
			shaderProgram.disableAttributeArray(colorLocation);
			shaderProgram.release();
		}
	}

protected:
	Data< std::vector<Point> > inputA, inputB;
	Data<PReal> width;
	Data< std::vector<Color> > color;
	Data< Shader > shader;

	graphics::ShaderProgram shaderProgram;
};

int RenderLineClass = RegisterObject<RenderLine>("Render/Line/Line").setDescription("Draw a line between 2 points");

//****************************************************************************//

class RenderPath : public Renderer
{
public:
	PANDA_CLASS(RenderPath, Renderer)

	RenderPath(PandaDocument* parent)
		: Renderer(parent)
		, input(initData("path", "Path to be drawn"))
		, lineWidth(initData("width", "Width of the line" ))
		, color(initData("color", "Color of the line"))
		, shader(initData("shader", "Shaders used during the rendering"))
	{
		addInput(input);
		addInput(lineWidth);
		addInput(color);
		addInput(shader);

		color.getAccessor().push_back(Color::black());
		lineWidth.getAccessor().push_back(0.0);

		shader.setWidgetData("Vertex;Fragment");
		auto shaderAcc = shader.getAccessor();
		shaderAcc->setSourceFromFile(Shader::ShaderType::Vertex, "shaders/PT_uniColor_noTex.v.glsl");
		shaderAcc->setSourceFromFile(Shader::ShaderType::Fragment, "shaders/PT_uniColor_noTex.f.glsl");
	}

	void render()
	{
		const std::vector<Path>& paths = input.getValue();
		const std::vector<Color>& listColor = color.getValue();
		const std::vector<PReal>& listWidth = lineWidth.getValue();

		int nbPaths = paths.size();
		int nbColor = listColor.size();
		int nbWidth = listWidth.size();

		if(nbPaths && nbColor && nbWidth)
		{
			if(!shader.getValue().apply(shaderProgram))
				return;

			if(nbColor < nbPaths) nbColor = 1;
			if(nbWidth < nbPaths) nbWidth = 1;

			shaderProgram.bind();
			shaderProgram.setUniformValueMat4("MVP", getMVPMatrix().constData());

			int vertexLocation = shaderProgram.attributeLocation("vertex");
			shaderProgram.enableAttributeArray(vertexLocation);

			int colorLocation = shaderProgram.uniformLocation("color");

			for(int i=0; i<nbPaths; ++i)
			{
				glLineWidth(listWidth[i % nbWidth]);

				shaderProgram.setUniformValueArray(colorLocation, listColor[i % nbColor].data(), 1, 4);

				const Path& path = paths[i];
				shaderProgram.setAttributeArray(vertexLocation, path.front().data(), 2);

				glDrawArrays(GL_LINE_STRIP, 0, path.size());
			}

			glLineWidth(0);
			shaderProgram.disableAttributeArray(vertexLocation);
			shaderProgram.release();
		}
	}

protected:
	Data< std::vector<Path> > input;
	Data< std::vector<PReal> > lineWidth;
	Data< std::vector<Color> > color;
	Data< Shader > shader;

	graphics::ShaderProgram shaderProgram;
};

int RenderPathClass = RegisterObject<RenderPath>("Render/Line/Path").setDescription("Draw a path");


//****************************************************************************//

class RenderGradientPath : public Renderer
{
public:
	PANDA_CLASS(RenderGradientPath, Renderer)

	RenderGradientPath(PandaDocument* parent)
		: Renderer(parent)
		, input(initData("path", "Path to be drawn"))
		, lineWidth(initData("width", "Width of the line" ))
		, gradient(initData("gradient", "Gradient applied to the line"))
		, shader(initData("shader", "Shaders used during the rendering"))
	{
		addInput(input);
		addInput(lineWidth);
		addInput(gradient);
		addInput(shader);

		gradient.getAccessor().push_back(Gradient::defaultGradient());
		lineWidth.getAccessor().push_back(0.0);

		shader.setWidgetData("Vertex;Fragment");
		auto shaderAcc = shader.getAccessor();
		shaderAcc->setSourceFromFile(Shader::ShaderType::Vertex, "shaders/PT_noColor_Tex.v.glsl");
		shaderAcc->setSourceFromFile(Shader::ShaderType::Fragment, "shaders/PT_noColor_Tex.f.glsl");
	}

	void update()
	{
		const std::vector<Path>& paths = input.getValue();
		int nbPaths = paths.size();

		texCoordsBuffer.clear();
		texCoordsBuffer.resize(nbPaths);

		pathLengths.clear();
		pathLengths.resize(nbPaths);

		for(int i=0; i<nbPaths; ++i)
		{
			const Path& path = paths[i];
			int nbPts = path.size();
			if(!nbPts)
				continue;
			texCoordsBuffer[i].resize(nbPts);
			auto& uvList = texCoordsBuffer[i];

			PReal length = 0;
			for(int j=0; j<nbPts-1; ++j)
			{
				length += (path[j+1] - path[j]).norm();
				uvList[j+1].x = length;
			}

			pathLengths[i] = length;

			if(length < 1e-3)
				uvList.clear();

			for(auto& uv : uvList)
				uv.x /= length;
		}
	}

	void render()
	{
		const std::vector<Path>& paths = input.getValue();
		const std::vector<Gradient>& listGradient = gradient.getValue();
		const std::vector<PReal>& listWidth = lineWidth.getValue();

		int nbPaths = paths.size();
		int nbGradient = listGradient.size();
		int nbWidth = listWidth.size();

		if(nbPaths && nbGradient && nbWidth)
		{
			if(!shader.getValue().apply(shaderProgram))
				return;

			if(nbGradient < nbPaths) nbGradient = 1;
			if(nbWidth < nbPaths) nbWidth = 1;

			shaderProgram.bind();
			shaderProgram.setUniformValueMat4("MVP", getMVPMatrix().constData());

			int vertexLocation = shaderProgram.attributeLocation("vertex");
			shaderProgram.enableAttributeArray(vertexLocation);

			int texCoordLocation = shaderProgram.attributeLocation("texCoord");
			shaderProgram.enableAttributeArray(texCoordLocation);

			shaderProgram.setUniformValue("tex0", 0);

			// Optimization when we use only one gradient
			if(nbGradient == 1)
			{
				PReal maxLength = *std::max_element(pathLengths.begin(), pathLengths.end());
				GLuint texture = GradientCache::getInstance()->getTexture(listGradient.front(), maxLength);

				glBindTexture(GL_TEXTURE_2D, texture);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D ,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

				for(int i=0; i<nbPaths; ++i)
				{
					glLineWidth(listWidth[i % nbWidth]);

					const Path& path = paths[i];
					const auto& uvList = texCoordsBuffer[i];
					if(uvList.empty())
						continue;
					shaderProgram.setAttributeArray(texCoordLocation, uvList.front().data(), 2);
					shaderProgram.setAttributeArray(vertexLocation, path.front().data(), 2);

					glDrawArrays(GL_LINE_STRIP, 0, path.size());
				}
			}
			else
			{
				for(int i=0; i<nbPaths; ++i)
				{
					glLineWidth(listWidth[i % nbWidth]);

					const Path& path = paths[i];
					const auto& uvList = texCoordsBuffer[i];
					if(uvList.empty())
						continue;
					shaderProgram.setAttributeArray(texCoordLocation, uvList.front().data(), 2);
					shaderProgram.setAttributeArray(vertexLocation, path.front().data(), 2);

					GLuint texture = GradientCache::getInstance()->getTexture(listGradient[i % nbGradient], pathLengths[i]);
					if(!texture)
						continue;
					glBindTexture(GL_TEXTURE_2D, texture);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D ,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

					glDrawArrays(GL_LINE_STRIP, 0, path.size());
				}
			}

			glLineWidth(0);
			shaderProgram.disableAttributeArray(vertexLocation);
			shaderProgram.disableAttributeArray("texCoord");
			shaderProgram.release();
		}
	}

protected:
	Data< std::vector<Path> > input;
	Data< std::vector<PReal> > lineWidth;
	Data< std::vector<Gradient> > gradient;
	Data< Shader > shader;

	std::vector<PReal> pathLengths;
	std::vector< std::vector<Point> > texCoordsBuffer;

	graphics::ShaderProgram shaderProgram;
};

int RenderGradientPathClass = RegisterObject<RenderGradientPath>("Render/Gradient/Path")
		.setName("Gradient path").setDescription("Draw a path");

} // namespace panda
