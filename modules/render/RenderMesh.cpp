#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/Color.h>
#include <panda/types/ImageWrapper.h>
#include <panda/types/Mesh.h>
#include <panda/types/Shader.h>
#include <panda/Renderer.h>

#include <QOpenGLShaderProgram>

namespace panda {

using types::Color;
using types::ImageWrapper;
using types::Point;
using types::Mesh;
using types::Shader;

class RenderMesh : public Renderer
{
public:
	PANDA_CLASS(RenderMesh, Renderer)

	RenderMesh(PandaDocument *parent)
		: Renderer(parent)
		, mesh(initData(&mesh, "mesh", "Mesh to render"))
		, color(initData(&color, "color", "Color of the points"))
		, shader(initData(&shader, "shader", "Shaders used during the rendering"))
	{
		addInput(&mesh);
		addInput(&color);
		addInput(&shader);

		color.getAccessor().push_back(Color::black());

		shader.setWidgetData("Vertex;Fragment");
		auto shaderAcc = shader.getAccessor();
		shaderAcc->setSourceFromFile(QOpenGLShader::Vertex, ":/shaders/PT_attColor_noTex.v.glsl");
		shaderAcc->setSourceFromFile(QOpenGLShader::Fragment, ":/shaders/PT_attColor_noTex.f.glsl");
	}

	void render()
	{
		const Mesh& inMesh = mesh.getValue();
		QVector<Color> listColor = color.getValue();

		int nbPts = inMesh.getNumberOfPoints();
		int nbTri = inMesh.getNumberOfTriangles();
		int nbColor = listColor.size();

		if(nbTri && nbColor)
		{
			if(nbColor < nbPts)
				listColor.fill(listColor[0], nbPts);

			if(!shader.getValue().apply(shaderProgram))
				return;

			shaderProgram.setUniformValue("MVP", getMVPMatrix());

			shaderProgram.enableAttributeArray("vertex");
			shaderProgram.setAttributeArray("vertex", inMesh.getPoints().front().data(), 2);

			shaderProgram.enableAttributeArray("color");
			shaderProgram.setAttributeArray("color", listColor.front().data(), 4);

			glDrawElements(GL_TRIANGLES, nbTri * 3, GL_UNSIGNED_INT, inMesh.getTriangles().data());

			shaderProgram.disableAttributeArray("vertex");
			shaderProgram.disableAttributeArray("color");
		}
	}

protected:
	Data< Mesh > mesh;
	Data< QVector<Color> > color;
	Data< Shader > shader;

	QOpenGLShaderProgram shaderProgram;
};

int RenderMeshClass = RegisterObject<RenderMesh>("Render/Filled/Mesh").setDescription("Draw a mesh");

//****************************************************************************//

class RenderMesh_Textured : public Renderer
{
public:
	PANDA_CLASS(RenderMesh_Textured, Renderer)

	RenderMesh_Textured(PandaDocument *parent)
		: Renderer(parent)
		, mesh(initData(&mesh, "mesh", "Mesh to render"))
		, uvCoords(initData(&uvCoords, "UV Coords", "Texture coordinates"))
		, texture(initData(&texture, "texture", "Texture applied to the mesh"))
		, shader(initData(&shader, "shader", "Shaders used during the rendering"))
	{
		addInput(&mesh);
		addInput(&uvCoords);
		addInput(&texture);
		addInput(&shader);

		shader.setWidgetData("Vertex;Fragment");
		auto shaderAcc = shader.getAccessor();
		shaderAcc->setSourceFromFile(QOpenGLShader::Vertex, ":/shaders/PT_noColor_Tex.v.glsl");
		shaderAcc->setSourceFromFile(QOpenGLShader::Fragment, ":/shaders/PT_noColor_Tex.f.glsl");
	}

	void render()
	{
		const Mesh& inMesh = mesh.getValue();
		QVector<Point> listCoords = uvCoords.getValue();
		const int texId = texture.getValue().getTextureId();

		int nbPts = inMesh.getNumberOfPoints();
		int nbTri = inMesh.getNumberOfTriangles();
		int nbCoords = listCoords.size();

		if(nbTri && (nbPts == nbCoords) && texId)
		{
			if(!shader.getValue().apply(shaderProgram))
				return;

			shaderProgram.setUniformValue("MVP", getMVPMatrix());

			shaderProgram.enableAttributeArray("vertex");
			shaderProgram.setAttributeArray("vertex", inMesh.getPoints().front().data(), 2);

			shaderProgram.enableAttributeArray("texCoord");
			shaderProgram.setAttributeArray("texCoord", listCoords.front().data(), 2);

			glBindTexture(GL_TEXTURE_2D, texId);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D ,GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			shaderProgram.setUniformValue("tex0", 0);

			glDrawElements(GL_TRIANGLES, nbTri * 3, GL_UNSIGNED_INT, inMesh.getTriangles().data());

			shaderProgram.disableAttributeArray("vertex");
			shaderProgram.disableAttributeArray("texCoord");
		}
	}

protected:
	Data< Mesh > mesh;
	Data< QVector<Point> > uvCoords;
	Data< ImageWrapper > texture;
	Data< Shader > shader;

	QOpenGLShaderProgram shaderProgram;
};

int RenderMesh_TexturedClass = RegisterObject<RenderMesh_Textured>("Render/Textured/Mesh")
		.setName("Textured mesh").setDescription("Draw a textured mesh");

} // namespace panda
