#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Renderer.h>
#include <panda/types/Color.h>
#include <panda/types/Point.h>
#include <panda/types/Gradient.h>
#include <panda/helper/GradientCache.h>

#include <QOpenGLFunctions_1_4>
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

		glEnableClientState(GL_VERTEX_ARRAY);

/*		glColor4fv(colorBuffer.front().data());
		glVertexPointer(2, GL_PREAL, 0, vertexBuffer.front().data());

		QOpenGLFunctions_1_4* glFuncs = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_1_4>();
		glFuncs->initializeOpenGLFunctions();
		glFuncs->glMultiDrawArrays(GL_TRIANGLE_FAN, firstBuffer.data(), countBuffer.data(), countBuffer.size());
*/
		int nb = countBuffer.size();
		for(int i=0; i<nb; ++i)
		{
			glColor4fv(colorBuffer[i].data());

			glVertexPointer(2, GL_PREAL, 0, vertexBuffer[firstBuffer[i]].data());
			glDrawArrays(GL_TRIANGLE_FAN, 0, countBuffer[i]);
		}

		glDisableClientState(GL_VERTEX_ARRAY);
	}

protected:
	Data< QVector<Point> > center;
	Data< QVector<PReal> > radius;
	Data< QVector<Color> > color;

	QVector<Point> vertexBuffer;
	QVector<GLint> firstBuffer;
	QVector<GLsizei> countBuffer;
	QVector<Color> colorBuffer;
};

int RenderDiskClass = RegisterObject<RenderDisk>("Render/Disk").setDescription("Draw a plain disk");

//****************************************************************************//

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

	void update()
	{
		const QVector<Point>& listCenter = center.getValue();
		const QVector<PReal>& listRadius = radius.getValue();

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

				for(int i=0; i<=nbSeg; ++i)
				{
					Point pt = Point(dir.x*ca+dir.y*sa, dir.y*ca-dir.x*sa);
					vertexBuffer[nbVertices + 1 + i] = valCenter + pt;
					texCoordsBuffer[nbVertices + 1 + i] = Point(1, 1);
					dir = pt;
				}
			}
		}

		cleanDirty();
	}

	void render()
	{
		const QVector<PReal>& listRadius = radius.getValue();
		const QVector<Gradient>& listGradient = gradient.getValue();

		int nbRadius = listRadius.size();
		int nbGradient = listGradient.size();
		int nbDisks = countBuffer.size();

		if(nbDisks && nbGradient)
		{
			if(nbRadius < nbDisks) nbRadius = 1;
			if(nbGradient < nbDisks) nbGradient = 1;

			glColor3f(1, 1, 1);

			glEnable(GL_TEXTURE_2D);
			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);

			for(int i=0; i<nbDisks; ++i)
			{
				GLuint texture = GradientCache::getInstance()->getTexture(listGradient[i % nbGradient], static_cast<int>(ceil(listRadius[i % nbRadius])));
				if(texture == -1)
					continue;
				glBindTexture(GL_TEXTURE_2D, texture);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D ,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

				glVertexPointer(2, GL_PREAL, 0, vertexBuffer[firstBuffer[i]].data());
				glTexCoordPointer(2, GL_PREAL, 0, texCoordsBuffer[firstBuffer[i]].data());
				glDrawArrays(GL_TRIANGLE_FAN, 0, countBuffer[i]);
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

	QVector<Point> vertexBuffer, texCoordsBuffer;
	QVector<GLint> firstBuffer;
	QVector<GLsizei> countBuffer;
};

int RenderDisk_GradientClass = RegisterObject<RenderDisk_Gradient>("Render/Gradient disk").setDescription("Draw a disk filled with a radial gradient");

} // namespace panda
