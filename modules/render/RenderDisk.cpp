#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Renderer.h>
#include <panda/types/Color.h>
#include <panda/types/Point.h>
#include <panda/types/Gradient.h>
#include <panda/helper/GradientCache.h>

#include <cmath>

namespace panda {

using types::Color;
using types::Point;
using types::Gradient;
using helper::GradientCache;

class RenderDisk : public Renderer
{
public:
	PANDA_CLASS(RenderDisk, Renderer)

	RenderDisk(PandaDocument *parent)
		: Renderer(parent)
		, center(initData(&center, "center", "Center position of the disk"))
		, radius(initData(&radius, "radius", "Radius of the disk"))
		, color(initData(&color, "color", "Color of the plain disk"))
	{
		addInput(&center);
		addInput(&radius);
		addInput(&color);

		center.getAccessor().push_back(Point(100, 100));
		radius.getAccessor().push_back(5.0);
		color.getAccessor().push_back(Color::black());
	}

	void update()
	{
		const QVector<Point>& listCenter = center.getValue();
		const QVector<PReal>& listRadius = radius.getValue();
		const QVector<Color>& listColor = color.getValue();

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
				firstBuffer.push_back(nbVertices * 2);
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

		glEnableClientState(GL_VERTEX_ARRAY);

		int nb = countBuffer.size();
		for(int i=0; i<nb; ++i)
		{
			glColor4fv(colorBuffer[i].data());

			glVertexPointer(2, GL_PREAL, 0, vertexBuffer[firstBuffer[i] / 2].data());
			glDrawArrays(GL_TRIANGLE_FAN, 0, countBuffer[i]);
		}
		glDisableClientState(GL_VERTEX_ARRAY);
	}

protected:
	Data< QVector<Point> > center;
	Data< QVector<PReal> > radius;
	Data< QVector<Color> > color;

	QVector<Point> vertexBuffer;
	QVector<int> firstBuffer, countBuffer;
	QVector<Color> colorBuffer;
};

int RenderDiskClass = RegisterObject<RenderDisk>("Render/Disk").setDescription("Draw a plain disk");

//*************************************************************************//

class RenderDisk_Gradient : public Renderer
{
public:
	PANDA_CLASS(RenderDisk_Gradient, Renderer)

	RenderDisk_Gradient(PandaDocument *parent)
		: Renderer(parent)
		, center(initData(&center, "center", "Center position of the disk"))
		, radius(initData(&radius, "radius", "Radius of the disk"))
		, gradient(initData(&gradient, "gradient", "Gradient used to fill the disk"))
	{
		addInput(&center);
		addInput(&radius);
		addInput(&gradient);

		center.getAccessor().push_back(Point(100, 100));
		radius.getAccessor().push_back(5.0);

		Gradient grad;
		grad.add(0, Color::black());
		grad.add(1, Color::white());
		gradient.getAccessor().push_back(grad);
	}

	void render()
	{
		const QVector<Point>& listCenter = center.getValue();
		const QVector<PReal>& listRadius = radius.getValue();
		const QVector<Gradient>& listGradient = gradient.getValue();

		int nbCenter = listCenter.size();
		int nbRadius = listRadius.size();
		int nbGradient = listGradient.size();

		if(nbCenter && nbRadius && nbGradient)
		{
			if(nbRadius < nbCenter) nbRadius = 1;
			if(nbGradient < nbCenter) nbGradient = 1;
			std::vector<PReal> vertices, texCoords;

			glEnable(GL_TEXTURE_2D);
			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);

			PReal PI2 = static_cast<PReal>(M_PI) * 2;
			for(int i=0; i<nbCenter; ++i)
			{
				PReal valRadius = listRadius[i % nbRadius];
				int nbSeg = static_cast<int>(floor(valRadius * PI2));
				if(nbSeg < 3) continue;
				int nbPoints = (nbSeg + 2) * 2;
				vertices.resize(nbPoints);
				texCoords.resize(nbPoints);

				GLuint texture = GradientCache::getInstance()->getTexture(listGradient[i % nbGradient], static_cast<int>(ceil(valRadius)));
				if(texture == -1)
					continue;
				glBindTexture(GL_TEXTURE_2D, texture);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D ,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

				Point valCenter = listCenter[i];
				vertices[0] = valCenter.x;
				vertices[1] = valCenter.y;
				texCoords[0] = 0; texCoords[1] = 0;

				PReal angle = PI2 / nbSeg;
				PReal ca = cos(angle), sa = sin(angle);
				Point dir = Point(valRadius, 0);

				for(int i=0; i<=nbSeg; ++i)
				{
					int index = (i+1)*2;
					Point pt = Point(dir.x*ca+dir.y*sa, dir.y*ca-dir.x*sa);
					vertices[index  ] = valCenter.x + pt.x;
					vertices[index+1] = valCenter.y + pt.y;
					dir = pt;
					texCoords[index] = 1; texCoords[index+1] = 0;
				}

				glVertexPointer(2, GL_PREAL, 0, vertices.data());
				glTexCoordPointer(2, GL_PREAL, 0, texCoords.data());
				glDrawArrays(GL_TRIANGLE_FAN, 0, nbSeg+2);
			}
			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			glDisable(GL_TEXTURE_2D);
		}
	}

protected:
	Data< QVector<Point> > center;
	Data< QVector<PReal> > radius;
	Data< QVector<Gradient> > gradient;
};

int RenderDisk_GradientClass = RegisterObject<RenderDisk_Gradient>("Render/Gradient disk").setDescription("Draw a disk filled with a radial gradient");

} // namespace panda
