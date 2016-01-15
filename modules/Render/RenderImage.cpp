#include <GL/glew.h>

#include <panda/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/object/Renderer.h>
#include <panda/types/Rect.h>
#include <panda/types/ImageWrapper.h>
#include <panda/types/Shader.h>
#include <panda/graphics/ShaderProgram.h>

namespace panda {

using types::Point;
using types::Rect;
using types::ImageWrapper;
using types::Shader;

class RenderImage : public Renderer
{
public:
	PANDA_CLASS(RenderImage, Renderer)

	RenderImage(PandaDocument* parent)
		: Renderer(parent)
		, image(initData("image", "Image to render on screen" ))
		, position(initData("position", "Position of the image"))
		, rotation(initData("rotation", "Rotation of the image"))
		, drawCentered(initData(0, "drawCentered", "If non zero use the center of the image, else use the top-left corner"))
		, shader(initData("shader", "Shaders used during the rendering"))
	{
		addInput(image);
		addInput(position);
		addInput(rotation);
		addInput(drawCentered);
		addInput(shader);

		drawCentered.setWidget("checkbox");

		position.getAccessor().push_back(Point(0, 0));

		shader.setWidgetData("Vertex;Fragment");
		auto shaderAcc = shader.getAccessor();
		shaderAcc->setSourceFromFile(Shader::ShaderType::Vertex, "shaders/PT_noColor_Tex.v.glsl");
		shaderAcc->setSourceFromFile(Shader::ShaderType::Fragment, "shaders/PT_noColor_Tex.f.glsl");

		m_texCoords[0*2+0] = 1; m_texCoords[0*2+1] = 1;
		m_texCoords[1*2+0] = 0; m_texCoords[1*2+1] = 1;
		m_texCoords[3*2+0] = 0; m_texCoords[3*2+1] = 0;
		m_texCoords[2*2+0] = 1; m_texCoords[2*2+1] = 0;
	}

	void update()
	{
		// Make sure we compute each image before the rendering
		const std::vector<ImageWrapper>& listImage = image.getValue();
		for(const ImageWrapper& image : listImage)
			image.getTextureId();
	}

	void drawTexture(GLuint texId, Rect area)
	{
		if(!texId)
			return;

		glBindTexture(GL_TEXTURE_2D, texId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D ,GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		m_verts[0*2+0] = area.right(); m_verts[0*2+1] = area.top();
		m_verts[1*2+0] = area.left();  m_verts[1*2+1] = area.top();
		m_verts[3*2+0] = area.left();  m_verts[3*2+1] = area.bottom();
		m_verts[2*2+0] = area.right(); m_verts[2*2+1] = area.bottom();

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

	void render()
	{
		const std::vector<ImageWrapper>& listImage = image.getValue();
		const std::vector<Point>& listPosition = position.getValue();
		const std::vector<PReal>& listRotation = rotation.getValue();

		bool centered = (drawCentered.getValue() != 0);

		int nbImage = listImage.size();
		int nbPosition = listPosition.size();
		int nbRotation = listRotation.size();

		if(nbImage && nbPosition)
		{
			if(nbImage < nbPosition) nbImage = 1;
			if(nbRotation && nbRotation < nbPosition) nbRotation = 1;

			if(!shader.getValue().apply(shaderProgram))
				return;

			shaderProgram.bind();
			const graphics::Mat4x4& MVP = getMVPMatrix();
			shaderProgram.setUniformValueMat4("MVP", MVP.data());

			shaderProgram.enableAttributeArray("vertex");
			shaderProgram.setAttributeArray("vertex", m_verts, 2);

			shaderProgram.enableAttributeArray("texCoord");
			shaderProgram.setAttributeArray("texCoord", m_texCoords, 2);

			shaderProgram.setUniformValue("tex0", 0);

			if(nbRotation)
			{
				if(centered)
				{
					for(int i=0; i<nbPosition; ++i)
					{
						const ImageWrapper& img = listImage[i % nbImage];
						auto s = img.size() / 2;
						if(s.empty()) continue;
						graphics::Mat4x4 tmpMVP = MVP;
						tmpMVP.translate(listPosition[i].x, listPosition[i].y, 0);
						tmpMVP.rotate(listRotation[i % nbRotation], 0, 0, 1);
						shaderProgram.setUniformValueMat4("MVP", tmpMVP.data());
						PReal w = static_cast<PReal>(s.width()), h = static_cast<PReal>(s.height());
						Rect area = Rect(-w, -h, w, h);
						drawTexture(img.getTextureId(), area);
					}
				}
				else
				{
					for(int i=0; i<nbPosition; ++i)
					{
						const ImageWrapper& img = listImage[i % nbImage];
						auto s = img.size();
						if(s.empty()) continue;
						graphics::Mat4x4 tmpMVP = MVP;
						tmpMVP.translate(listPosition[i].x, listPosition[i].y, 0);
						tmpMVP.rotate(listRotation[i % nbRotation], 0, 0, 1);
						shaderProgram.setUniformValueMat4("MVP", tmpMVP.data());
						PReal w = static_cast<PReal>(s.width()), h = static_cast<PReal>(s.height());
						Rect area = Rect(0, 0, w, h);
						drawTexture(img.getTextureId(), area);
					}
				}
			}
			else
			{
				if(centered)
				{
					for(int i=0; i<nbPosition; ++i)
					{
						const ImageWrapper& img = listImage[i % nbImage];
						auto s = img.size() / 2;
						if(s.empty()) continue;
						PReal w = static_cast<PReal>(s.width()), h = static_cast<PReal>(s.height());
						Rect area = Rect(listPosition[i].x - w,
										 listPosition[i].y - h,
										 listPosition[i].x + w,
										 listPosition[i].y + h);
						drawTexture(img.getTextureId(), area);
					}
				}
				else
				{
					for(int i=0; i<nbPosition; ++i)
					{
						const ImageWrapper& img = listImage[i % nbImage];
						auto s = img.size();
						if(s.empty()) continue;
						PReal w = static_cast<PReal>(s.width()), h = static_cast<PReal>(s.height());
						Rect area(listPosition[i], w, h);
						drawTexture(img.getTextureId(), area);
					}
				}
			}

			shaderProgram.disableAttributeArray("vertex");
			shaderProgram.disableAttributeArray("texCoord");
			shaderProgram.release();
		}
	}

protected:
	Data< std::vector<ImageWrapper> > image;
	Data< std::vector<Point> > position;
	Data< std::vector<PReal> > rotation;
	Data< int > drawCentered;
	Data< Shader > shader;

	GLfloat m_verts[8], m_texCoords[8];

	graphics::ShaderProgram shaderProgram;
};

int RenderImageClass = RegisterObject<RenderImage>("Render/Textured/Image").setDescription("Renders an image");

} // namespace panda
