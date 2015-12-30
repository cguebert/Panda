#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Renderer.h>
#include <panda/types/Color.h>
#include <panda/types/Rect.h>

#include <QOpenGLFramebufferObject>
#include <QOpenGLPaintDevice>
#include <QOpenGLFunctions>
#include <QPainter>
#include <QFont>

namespace panda {

using types::Color;
using types::Rect;

class RenderText : public Renderer
{
public:
	PANDA_CLASS(RenderText, Renderer)

	RenderText(PandaDocument* parent)
		: Renderer(parent)
		, text(initData("text", "Text to be drawn"))
		, font(initData("font", "Font to use for the text rendering"))
		, rect(initData("rectangle", "Rectangle in which to draw the text"))
		, color(initData("color", "Color of the text"))
		, alignH(initData(2, "align H", "Horizontal alignment of the text"))
		, alignV(initData(2, "align V", "Vertical alignment of the text"))
	{
		addInput(text);
		addInput(font);
		addInput(rect);
		addInput(color);
		addInput(alignH);
		addInput(alignV);

		font.setWidget("font");
		QFont tmp;
		font.setValue(tmp.toString().toStdString());

		alignH.setWidget("enum");
		alignH.setWidgetData("Left;Right;Center;Justify");

		alignV.setWidget("enum");
		alignV.setWidgetData("Top;Bottom;Center;Baseline");

		color.getAccessor().push_back(Color::black());
	}

	void render()
	{
		const auto& listText = text.getValue();
		const auto& listRect = rect.getValue();
		const auto& listColor = color.getValue();

		int nbText = listText.size();
		int nbRect = listRect.size();
		int nbColor = listColor.size();

		if(nbText && nbRect && nbColor)
		{
			const int alignHVals[] = {Qt::AlignLeft, Qt::AlignRight, Qt::AlignHCenter, Qt::AlignJustify};
			const int alignVVals[] = {Qt::AlignTop, Qt::AlignBottom, Qt::AlignVCenter, Qt::AlignBaseline};
			int alignHIndex = qBound(0, alignH.getValue(), 3);
			int alignVIndex = qBound(0, alignV.getValue(), 3);
			int alignment = alignHVals[alignHIndex] | alignVVals[alignVIndex];

			QSize renderSize = m_parentDocument->getRenderSize();
			if(!renderFrameBuffer || renderFrameBuffer->size() != renderSize)
			{
				QOpenGLFramebufferObjectFormat fmt;
				fmt.setSamples(16);
				fmt.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
				renderFrameBuffer.reset(new QOpenGLFramebufferObject(renderSize, fmt));
				displayFrameBuffer.reset(new QOpenGLFramebufferObject(renderSize));
			}

			QOpenGLFunctions functions(QOpenGLContext::currentContext());

			GLint previousFBO;
			glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &previousFBO);
			renderFrameBuffer->bind();

			glClearColor(0, 0, 0, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			glViewport(0, 0, renderSize.width(), renderSize.height());

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(0, renderSize.width(), renderSize.height(), 0, -10, 10);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			QOpenGLPaintDevice device(renderSize);
			QPainter painter;
			painter.begin(&device);
			painter.setRenderHints(QPainter::Antialiasing | QPainter::HighQualityAntialiasing);

			painter.setBrush(Qt::NoBrush);
			painter.setPen(Qt::NoPen);
			QFont theFont;
			theFont.fromString(QString::fromStdString(font.getValue()));
			painter.setFont(theFont);

			if(nbText < nbRect) nbText = 1;
			if(nbColor < nbRect) nbColor = 1;

			for(int i=0; i<nbRect; ++i)
			{
				const Color& c = listColor[i % nbColor];
				QColor col = QColor::fromRgbF(c.r, c.g, c.b, c.a);
				painter.setPen(QPen(col));
				Rect r = listRect[i];
				QRectF rect = QRectF(r.left(), r.top(), r.width(), r.height());
				painter.drawText(rect, alignment, QString::fromStdString(listText[i % nbText]));
			}

			painter.end();
			glClear(GL_STENCIL_BUFFER_BIT);
			glDisable(GL_STENCIL_TEST);
			glDisable(GL_DEPTH_TEST);
			glDepthMask(GL_FALSE);

			renderFrameBuffer->release();
			QOpenGLFramebufferObject::blitFramebuffer(displayFrameBuffer.data(), renderFrameBuffer.data());
			functions.glBindFramebuffer(GL_FRAMEBUFFER, previousFBO);

			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, displayFrameBuffer->texture());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D ,GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glEnableClientState(GL_VERTEX_ARRAY);
			GLfloat verts[8];
			glVertexPointer(2, GL_FLOAT, 0, verts);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			GLfloat texCoords[8];
			glTexCoordPointer(2, GL_FLOAT, 0, texCoords);

			verts[0*2+0] = renderSize.width(); verts[0*2+1] = 0;
			verts[1*2+0] = 0; verts[1*2+1] = 0;
			verts[3*2+0] = 0; verts[3*2+1] = renderSize.height();
			verts[2*2+0] = renderSize.width(); verts[2*2+1] = renderSize.height();

			texCoords[0*2+0] = 1; texCoords[0*2+1] = 1;
			texCoords[1*2+0] = 0; texCoords[1*2+1] = 1;
			texCoords[3*2+0] = 0; texCoords[3*2+1] = 0;
			texCoords[2*2+0] = 1; texCoords[2*2+1] = 0;

			glColor4f(1,1,1,1);

			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			glDisable(GL_TEXTURE_2D);
		}
	}

protected:
	Data< std::vector<std::string> > text;
	Data< std::string > font;
	Data< std::vector<Rect> > rect;
	Data< std::vector<Color> > color;
	Data< int > alignH, alignV;

	QSharedPointer<QOpenGLFramebufferObject> renderFrameBuffer, displayFrameBuffer;
};

int RenderTextClass = RegisterObject<RenderText>("Render/Text").setDescription("Draw some text");

} // namespace panda
