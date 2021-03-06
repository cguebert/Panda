#include <ui/widget/AnimationDataWidgetDialog.h>
#include <ui/widget/DataWidgetFactory.h>
#include <ui/widget/OpenDialogDataWidget.h>
#include <ui/widget/ListDataWidgetDialog.h>
#include <ui/widget/SimpleDataWidget.h>

#include <panda/types/Color.h>

#include <QtWidgets>

using panda::types::Animation;
using panda::types::Color;

static QColor toQColor(const Color& c)
{
	Color bc = c.bounded();
	return QColor::fromRgbF(bc.r, bc.g, bc.b, bc.a);
}

static Color fromQColor(const QColor& c)
{ return Color(c.redF(), c.greenF(), c.blueF(), c.alphaF()); }

template<>
class DataWidgetContainer< Color > : public BaseOpenDialogObject, public ObjectWithPreview
{
protected:
	typedef Color value_type;
	QWidget* container;
	PreviewView* preview;
	Color theColor;

public:
	DataWidgetContainer() : container(nullptr), preview(nullptr) {}

	QWidget* createWidgets(QWidget* parent, bool readOnly)
	{
		container = new QWidget(parent);
		preview = new PreviewView(this);

		QPushButton* pushButton = new QPushButton("...");
		pushButton->setEnabled(!readOnly);
		pushButton->setMaximumWidth(40);

		QHBoxLayout* layout = new QHBoxLayout(container);
		layout->setMargin(0);
		layout->addWidget(preview, 1);
		layout->addWidget(pushButton);
		container->setLayout(layout);

		QObject::connect(pushButton, SIGNAL(clicked()), this, SLOT(onShowDialog()) );
		return container;
	}
	QWidget* createWidgets(BaseDataWidget* parent, bool readOnly)
	{
		QWidget* parentWidget = dynamic_cast<QWidget*>(parent);
		QWidget* widget = createWidgets(parentWidget, readOnly);
		QObject::connect(this, SIGNAL(editingFinished()), parent, SLOT(setWidgetDirty()) );
		return widget;
	}
	void readFromData(const value_type& v)
	{
		if(theColor != v)
		{
			theColor = v;
			updatePreview();
		}
	}
	void writeToData(value_type& v)
	{
		if(v != theColor)
		{
			v = theColor;
			updatePreview();
		}
	}
	virtual void onShowDialog()
	{
		QColor tmp = QColorDialog::getColor(toQColor(theColor), container, "", QColorDialog::ShowAlphaChannel);
		if(tmp.isValid())
		{
			theColor = fromQColor(tmp);
			emit editingFinished();
		}
	}
	void updatePreview()
	{
		preview->update();
	}
	void draw(QPainter& painter, QSize size)
	{
		static QPixmap pm;
		if(pm.isNull())
		{
			int s = ceil(size.height() / 2.0);
			pm = QPixmap(2*s, 2*s);
			QColor c1 = container->palette().color(QPalette::Midlight);
			QColor c2 = container->palette().color(QPalette::Dark);
			QPainter pmp(&pm);
			pmp.fillRect(0, 0, s, s, c1);
			pmp.fillRect(s, s, s, s, c1);
			pmp.fillRect(0, s, s, s, c2);
			pmp.fillRect(s, 0, s, s, c2);
		}
		painter.fillRect(0, 0, size.width(), size.height(), QBrush(pm));
		painter.fillRect(0, 0, size.width(), size.height(), QBrush(toQColor(theColor)));
	}
};

//****************************************************************************//

template<>
static QString FlatDataTrait<Color>::toString(const value_type& d)
{ return QColor::fromRgba(d.toHex()).name(QColor::HexArgb).toUpper(); }

//****************************************************************************//

RegisterWidget<SimpleDataWidget<Color> > DWClass_color("default");
RegisterWidget<OpenDialogDataWidget<std::vector<Color>, ListDataWidgetDialog<std::vector<Color> > > > DWClass_colors_list_generic("generic");
RegisterWidget<OpenDialogDataWidget<Animation<Color>, AnimationDataWidgetDialog<Animation<Color> > > > DWClass_colors_animation("default");

