#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Renderer.h>
#include <panda/types/Rect.h>
#include <panda/types/ImageWrapper.h>
#include <panda/types/Shader.h>

#include <QOpenGLShaderProgram>

namespace panda {

using types::Point;
using types::Rect;
using types::ImageWrapper;
using types::Shader;

class RenderImage : public Renderer
{
public:
	PANDA_CLASS(RenderImage, Renderer)

	RenderImage(PandaDocument *parent)
		: Renderer(parent)
		, image(initData(&image, "image", "Image to render on screen" ))
		, center(initData(&center, "center", "Center position of the image"))
		, rotation(initData(&rotation, "rotation", "Rotation of the image"))
		, drawCentered(initData(&drawCentered, 0, "drawCentered", "If non zero use the center of the image, else use the top-left corner"))
		, shader(initData(&shader, "shader", "Shaders used during the rendering"))
	{
		addInput(&image);
		addInput(&center);
		addInput(&rotation);
		addInput(&drawCentered);
		addInput(&shader);

		drawCentered.setWidget("checkbox");

		center.getAccessor().push_back(Point(0, 0));

		shader.setWidgetData("Vertex;Fragment");
		auto shaderAcc = shader.getAccessor();
		shaderAcc->addSource(QOpenGLShader::Vertex,
							 "uniform mat4 MVP;\n"
							 "void main(void)\n"
							 "{\n"
							 "  gl_TexCoord[0] = gl_MultiTexCoord0;\n"
							 "  gl_Position = MVP * vec4(gl_Vertex.xy, 0, 1);\n"
							 "}"
							);

		shaderAcc->addSource(QOpenGLShader::Fragment,
							 "uniform sampler2D tex0;\n"
							 "void main(void)\n"
							 "{\n"
							 "  gl_FragColor = texture(tex0, gl_TexCoord[0].st);\n"
							 "}"
							);
	}

	void drawTexture(GLuint texId, Rect area)
	{
		if(!texId)
			return;

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texId);
		glEnableClientState(GL_VERTEX_ARRAY);
		GLfloat verts[8];
		glVertexPointer(2, GL_FLOAT, 0, verts);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		GLfloat texCoords[8];
		glTexCoordPointer(2, GL_FLOAT, 0, texCoords);

		verts[0*2+0] = area.right(); verts[0*2+1] = area.top();
		verts[1*2+0] = area.left(); verts[1*2+1] = area.top();
		verts[3*2+0] = area.left(); verts[3*2+1] = area.bottom();
		verts[2*2+0] = area.right(); verts[2*2+1] = area.bottom();

		texCoords[0*2+0] = 1; texCoords[0*2+1] = 1;
		texCoords[1*2+0] = 0; texCoords[1*2+1] = 1;
		texCoords[3*2+0] = 0; texCoords[3*2+1] = 0;
		texCoords[2*2+0] = 1; texCoords[2*2+1] = 0;

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisable(GL_TEXTURE_2D);
	}

	void render()
	{
		const QVector<ImageWrapper>& listImage = image.getValue();
		const QVector<Point>& listCenter = center.getValue();
		const QVector<PReal>& listRotation = rotation.getValue();

		bool centered = (drawCentered.getValue() != 0);

		int nbImage = listImage.size();
		int nbCenter = listCenter.size();
		int nbRotation = listRotation.size();

		if(nbImage && nbCenter)
		{
			if(nbImage < nbCenter) nbImage = 1;
			if(nbRotation && nbRotation < nbCenter) nbRotation = 1;

			shader.getValue().apply(shaderProgram);
			if(!shaderProgram.isLinked())
				return;

			shaderProgram.bind();
			const QMatrix4x4& MVP = getMVPMatrix();
			shaderProgram.setUniformValue("MVP", getMVPMatrix());

			glColor4f(1, 1, 1, 1);

			if(nbRotation)
			{
				if(centered)
				{
					for(int i=0; i<nbCenter; ++i)
					{
						const ImageWrapper& img = listImage[i % nbImage];
						QSize s = img.size() / 2;
						if(!s.isValid()) continue;
						QMatrix4x4 tmpMVP = MVP;
						tmpMVP.translate(listCenter[i].x, listCenter[i].y, 0);
						tmpMVP.rotate(listRotation[i % nbRotation], 0, 0, 1);
						shaderProgram.setUniformValue("MVP", tmpMVP);
						Rect area = Rect(-s.width(), -s.height(),
											 s.width(), s.height());
						drawTexture(img.getTexture(), area);
					}
				}
				else
				{
					for(int i=0; i<nbCenter; ++i)
					{
						const ImageWrapper& img = listImage[i % nbImage];
						QSize s = img.size();
						if(!s.isValid()) continue;
						QMatrix4x4 tmpMVP = MVP;
						tmpMVP.translate(listCenter[i].x, listCenter[i].y, 0);
						tmpMVP.rotate(listRotation[i % nbRotation], 0, 0, 1);
						shaderProgram.setUniformValue("MVP", tmpMVP);
						Rect area = Rect(0, 0, s.width(), s.height());
						drawTexture(img.getTexture(), area);
					}
				}
			}
			else
			{
				if(centered)
				{
					for(int i=0; i<nbCenter; ++i)
					{
						const ImageWrapper& img = listImage[i % nbImage];
						QSize s = img.size() / 2;
						if(!s.isValid()) continue;
						Rect area = Rect(listCenter[i].x - s.width(),
										 listCenter[i].y - s.height(),
										 listCenter[i].x + s.width(),
										 listCenter[i].y + s.height());
						drawTexture(img.getTexture(), area);
					}
				}
				else
				{
					for(int i=0; i<nbCenter; ++i)
					{
						const ImageWrapper& img = listImage[i % nbImage];
						QSize s = img.size();
						if(!s.isValid()) continue;
						Rect area(listCenter[i], s.width(), s.height());
						drawTexture(img.getTexture(), area);
					}
				}
			}

			shaderProgram.release();
		}
	}

protected:
	Data< QVector<ImageWrapper> > image;
	Data< QVector<Point> > center;
	Data< QVector<PReal> > rotation;
	Data< int > drawCentered;
	Data< Shader > shader;

	QOpenGLShaderProgram shaderProgram;
};

int RenderImageClass = RegisterObject<RenderImage>("Render/Image").setDescription("Renders an image");

} // namespace panda
