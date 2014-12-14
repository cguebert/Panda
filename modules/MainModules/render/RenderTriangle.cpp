#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/Color.h>
#include <panda/types/Mesh.h>
#include <panda/types/Shader.h>
#include <panda/Renderer.h>

#include <QOpenGLShaderProgram>

namespace panda {

using types::Color;
using types::Point;
using types::Mesh;
using types::Shader;

class RenderTriangle : public Renderer
{
public:
	PANDA_CLASS(RenderTriangle, Renderer)

	RenderTriangle(PandaDocument* parent)
		: Renderer(parent)
		, mesh(initData(&mesh, "mesh", "Triangle to render"))
		, color(initData(&color, "color", "Color of the triangle"))
		, shader(initData(&shader, "shader", "Shaders used during the rendering"))
	{
		addInput(&mesh);
		addInput(&color);
		addInput(&shader);

		color.getAccessor().push_back(Color::black());

		shader.setWidgetData("Vertex;Fragment");
		auto shaderAcc = shader.getAccessor();
		shaderAcc->setSourceFromFile(QOpenGLShader::Vertex, "shaders/PT_uniColor_noTex.v.glsl");
		shaderAcc->setSourceFromFile(QOpenGLShader::Fragment, "shaders/PT_uniColor_noTex.f.glsl");
	}

	void render()
	{
		const Mesh& inMesh = mesh.getValue();
		QVector<Color> listColor = color.getValue();

		int nbTri = inMesh.getNumberOfTriangles();
		int nbColor = listColor.size();

		if(nbTri && nbColor)
		{
			if(nbColor < nbTri) nbColor = 1;

			if(!shader.getValue().apply(shaderProgram))
				return;

			shaderProgram.setUniformValue("MVP", getMVPMatrix());

			shaderProgram.enableAttributeArray("vertex");
			shaderProgram.setAttributeArray("vertex", inMesh.getPoints().front().data(), 2);

			int colorLocation = shaderProgram.uniformLocation("color");

			const auto& triangles = inMesh.getTriangles();

			for(int i=0; i<nbTri; ++i)
			{
				auto color = listColor[i % nbColor];
				shaderProgram.setUniformValue(colorLocation, color.r, color.g, color.b, color.a);

				glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, triangles[i].data());
			}

			shaderProgram.disableAttributeArray("vertex");
		}
	}

protected:
	Data< Mesh > mesh;
	Data< QVector<Color> > color;
	Data< Shader > shader;

	QOpenGLShaderProgram shaderProgram;
};

int RenderTriangleClass = RegisterObject<RenderTriangle>("Render/Filled/Triangle").setDescription("Draw a triangle");

} // namespace panda