#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Renderer.h>
#include <panda/types/Gradient.h>
#include <panda/helper/GradientCache.h>

#include <QPointF>
#include <QOpenGLFunctions_3_3_Core>

using panda::types::Gradient;
using panda::helper::GradientCache;

namespace panda {

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

		center.getAccessor().push_back(QPointF(100, 100));
		radius.getAccessor().push_back(5.0);
		color.getAccessor().push_back(QColor(0,0,0));

		QOpenGLContext* context = QOpenGLContext::currentContext();
		functions = context->versionFunctions<QOpenGLFunctions_3_3_Core>();
		functions->initializeOpenGLFunctions();
	}

	void render()
	{
		const QVector<QPointF>& listCenter = center.getValue();
		const QVector<double>& listRadius = radius.getValue();
		const QVector<QColor>& listColor = color.getValue();

		int nbCenter = listCenter.size();
		int nbRadius = listRadius.size();
		int nbColor = listColor.size();

		if(nbCenter && nbRadius && nbColor)
		{
			if(nbRadius < nbCenter) nbRadius = 1;
			if(nbColor < nbCenter) nbColor = 1;
			QVector<float> vertices;
			QVector<int> first, count;
			first.resize(nbCenter);
			count.resize(nbCenter);

			for(int i=0; i<nbCenter; ++i)
			{
				first[i] = vertices.size() / 2;
				QColor valCol = listColor[i % nbColor];
				glColor4ub(valCol.red(), valCol.green(), valCol.blue(), valCol.alpha());

				float valRadius = listRadius[i % nbRadius];
				int nbSeg = static_cast<int>(floor(valRadius * M_PI * 2));
				if(nbSeg < 3) continue;

				QPointF valCenter = listCenter[i];
				vertices.push_back(valCenter.x());
				vertices.push_back(valCenter.y());

				for(int j=0; j<=nbSeg; ++j)
				{
					float t = j / static_cast<float>(nbSeg) * 2 * M_PI;
					vertices.push_back(valCenter.x() + cos(t) * valRadius);
					vertices.push_back(valCenter.y() + sin(t) * valRadius);
				}

				count[i] = vertices.size() / 2 - first[i];
			}

			glEnableClientState(GL_VERTEX_ARRAY);
			glVertexPointer(2, GL_FLOAT, 0, vertices.data());
			functions->glMultiDrawArrays(GL_TRIANGLE_FAN, first.data(), count.data(), nbCenter);
			glDisableClientState(GL_VERTEX_ARRAY);
		}
	}

protected:
	Data< QVector<QPointF> > center;
	Data< QVector<double> > radius;
	Data< QVector<QColor> > color;

	QOpenGLFunctions_3_3_Core* functions;
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

		center.getAccessor().push_back(QPointF(100, 100));
		radius.getAccessor().push_back(5.0);

		Gradient grad;
		grad.add(0, QColor());
		grad.add(1, QColor(255, 255, 255));
		gradient.getAccessor().push_back(grad);
	}

	void render()
	{
		const QVector<QPointF>& listCenter = center.getValue();
		const QVector<double>& listRadius = radius.getValue();
		const QVector<Gradient>& listGradient = gradient.getValue();

		int nbCenter = listCenter.size();
		int nbRadius = listRadius.size();
		int nbGradient = listGradient.size();

		if(nbCenter && nbRadius && nbGradient)
		{
			if(nbRadius < nbCenter) nbRadius = 1;
			if(nbGradient < nbCenter) nbGradient = 1;
			std::vector<double> vertices, texCoords;

			glEnable(GL_TEXTURE_2D);
			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);

			for(int i=0; i<nbCenter; ++i)
			{
				double valRadius = listRadius[i % nbRadius];
				int nbSeg = static_cast<int>(floor(valRadius * M_PI * 2));
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

				QPointF valCenter = listCenter[i];
				vertices[0] = valCenter.x();
				vertices[1] = valCenter.y();
				texCoords[0] = 0; texCoords[1] = 0;

				for(int i=0; i<=nbSeg; ++i)
				{
					double t = i / static_cast<double>(nbSeg) * 2 * M_PI;
					int index = (i+1)*2;
					vertices[index] = valCenter.x() + cos(t) * valRadius;
					vertices[index+1] = valCenter.y() + sin(t) * valRadius;
					texCoords[index] = 1; texCoords[index+1] = 0;
				}

				glVertexPointer(2, GL_DOUBLE, 0, vertices.data());
				glTexCoordPointer(2, GL_DOUBLE, 0, texCoords.data());
				glDrawArrays(GL_TRIANGLE_FAN, 0, nbSeg+2);
			}
			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			glDisable(GL_TEXTURE_2D);
		}
	}

protected:
	Data< QVector<QPointF> > center;
	Data< QVector<double> > radius;
	Data< QVector<Gradient> > gradient;
};

int RenderDisk_GradientClass = RegisterObject<RenderDisk_Gradient>("Render/Gradient disk").setDescription("Draw a disk filled with a radial gradient");

} // namespace panda
