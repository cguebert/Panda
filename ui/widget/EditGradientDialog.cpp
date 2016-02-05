#include <ui/widget/ListDataWidgetDialog.h>
#include <ui/widget/SimpleDataWidget.h>
#include <ui/widget/OpenDialogDataWidget.h>
#include <ui/widget/EditGradientDialog.h>
#include <ui/widget/AnimationDataWidgetDialog.h>
#include <ui/widget/DataWidgetFactory.h>

#include <panda/types/Gradient.h>

#include <QtWidgets>

using panda::types::Color;
using panda::types::Gradient;
using panda::types::Animation;

static QColor toQColor(const Color& c)
{
	Color bc = c.bounded();
	return QColor::fromRgbF(bc.r, bc.g, bc.b, bc.a);
}

static Color fromQColor(const QColor& c)
{ return Color(c.redF(), c.greenF(), c.blueF(), c.alphaF()); }

ColorPreviewWidget::ColorPreviewWidget(QWidget* parent)
	: QWidget(parent)
{}

void ColorPreviewWidget::setColor(QColor color)
{
	theColor = color;
	update();
}

void ColorPreviewWidget::paintEvent(QPaintEvent *)
{
	QStylePainter painter(this);
	if(isEnabled())
	{
		static QPixmap pm;
		if(pm.isNull())
		{
			int s = ceil(height() / 2.0);
			pm = QPixmap(2*s, 2*s);
			QColor c1 = palette().color(QPalette::Midlight);
			QColor c2 = palette().color(QPalette::Dark);
			QPainter pmp(&pm);
			pmp.fillRect(0, 0, s, s, c1);
			pmp.fillRect(s, s, s, s, c1);
			pmp.fillRect(0, s, s, s, c2);
			pmp.fillRect(s, 0, s, s, c2);
		}
		painter.fillRect(0, 0, width(), height(), QBrush(pm));
		painter.fillRect(0, 0, width(), height(), QBrush(theColor));
	}
}

//****************************************************************************//


EditGradientView::EditGradientView(EditGradientDialog *dlg, Gradient::GradientStops &stops)
	: dialog(dlg)
	, stops(stops)
	, selected(-1)
	, moving(false)
{
}

void EditGradientView::paintEvent(QPaintEvent *)
{
	const int hm = 5;	// horizontal margin
	const int vm = 1;	// vertical margin
	const int gradH = 30;
	const int gradW = width() - 2*hm;
	const int vs = 8;	// vignette size

	QStylePainter painter(this);
	static QPixmap pm;
	if(pm.isNull())
	{
		int s = ceil((gradH+1) / 2.0);
		pm = QPixmap(2*s, 2*s);
		QColor c1 = palette().color(QPalette::Midlight);
		QColor c2 = palette().color(QPalette::Dark);
		QPainter pmp(&pm);
		pmp.fillRect(0, 0, s, s, c1);
		pmp.fillRect(s, s, s, s, c1);
		pmp.fillRect(0, s, s, s, c2);
		pmp.fillRect(s, 0, s, s, c2);
	}
	painter.setPen(palette().color(QPalette::Text));
	painter.drawRect(hm-1, vm-1, gradW+1, gradH+1);
	painter.fillRect(hm, vm, gradW, gradH, pm);

	QLinearGradient grad(0, 0, gradW, 0);
	for(const auto&s : stops)
		grad.setColorAt(s.first, toQColor(s.second));
	painter.fillRect(hm, vm, gradW, gradH, grad);

	paths.clear();
	for(int i=0, nb=stops.size(); i<nb; ++i)
	{
		auto stop = stops[i];
		int x = hm + gradW * stop.first;
		int y = vm + gradH;

		if(i == selected)
			painter.setPen(QPen(palette().color(QPalette::Text), 2));
		else
			painter.setPen(palette().color(QPalette::Text));

		QColor col = toQColor(stop.second);
		col.setAlphaF(1.0);
		painter.setBrush(col);
		QPainterPath path;
		path.moveTo(x, y);
		path.lineTo(x+vs/2, y+vs);
		path.lineTo(x+vs, y+vs);
		path.lineTo(x+vs, y+3*vs);
		path.lineTo(x-vs, y+3*vs);
		path.lineTo(x-vs, y+vs);
		path.lineTo(x-vs/2, y+vs);
		path.lineTo(x, y);
		painter.drawPath(path);
		paths.push_back(path);
	}
}

void EditGradientView::mousePressEvent(QMouseEvent* event)
{
	int prevSelected = selected;
	selected = -1;
	for(int i=0, nb=paths.size(); i<nb; ++i)
	{
		if(paths[i].contains(event->pos()))
		{
			selected = i;
			break;
		}
	}

	dialog->setSelected(selected);
	if(selected != prevSelected)
		update();

	if(selected != -1)
	{
		moving = true;
		initialMouseX = event->x();
		initialPos = prevPos = stops[selected].first;
	}
}

void EditGradientView::mouseMoveEvent(QMouseEvent* event)
{
	if(moving)
	{
		int dx = event->x() - initialMouseX;
		float dfx = dx / static_cast<float>(width() - 10);
		float pos = qBound<float>(0.0, initialPos + dfx, 1.0);
		if(pos != prevPos)
		{
			stops[selected].first = pos;
			dialog->positionHasChanged();
			update();
			prevPos = pos;
		}
	}
}

void EditGradientView::mouseReleaseEvent(QMouseEvent*)
{
	moving = false;
}

void EditGradientView::setSelected(int sel)
{
	selected = sel;
}

//****************************************************************************//

EditGradientDialog::EditGradientDialog(Gradient grad, QWidget* parent)
	: QDialog(parent)
	, stops(grad.getStopsForEdit())
	, extend(grad.getExtend())
	, selected(0)
{
	QVBoxLayout* vLayout = new QVBoxLayout;
	view = new EditGradientView(this, stops);
	vLayout->addWidget(view);
	view->setMinimumSize(250, 60);
	view->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

	QPushButton* addButton = new QPushButton(tr("Add color"));
	connect(addButton, SIGNAL(clicked()), this, SLOT(addStop()));
	removeButton = new QPushButton(tr("Remove color"));
	connect(removeButton, SIGNAL(clicked()), this, SLOT(removeStop()));
	QHBoxLayout* gradButtonsLayout = new QHBoxLayout;
	gradButtonsLayout->addStretch();
	gradButtonsLayout->addWidget(addButton);
	gradButtonsLayout->addWidget(removeButton);
	vLayout->addLayout(gradButtonsLayout);

	QLabel* posLabel = new QLabel(tr("position:"));
	posEdit = new QLineEdit;
	connect(posEdit, SIGNAL(editingFinished()), this, SLOT(changePosition()));
	QHBoxLayout* posLayout = new QHBoxLayout;
	posLayout->addWidget(posLabel);
	posLayout->addWidget(posEdit);
	vLayout->addLayout(posLayout);

	QLabel* colorLabel = new QLabel(tr("color:"));
	colorPreview = new ColorPreviewWidget(this);
	colorPreview->setEnabled(false);
	colorButton = new QPushButton(tr("Choose"));
	connect(colorButton, SIGNAL(clicked()), this, SLOT(chooseColor()));
	QHBoxLayout* colorLayout = new QHBoxLayout;
	colorLayout->addWidget(colorLabel);
	colorLayout->addWidget(colorPreview, 1);
	colorLayout->addWidget(colorButton);
	vLayout->addLayout(colorLayout);

	QLabel* extendLabel = new QLabel(tr("Extend method:"));
	QComboBox* extendBox = new QComboBox;
	extendBox->addItem("Pad");
	extendBox->addItem("Repeat");
	extendBox->addItem("Reflect");
	extendBox->setCurrentIndex(extend);
	connect(extendBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeExtend(int)));
	QHBoxLayout* extendLayout = new QHBoxLayout;
	extendLayout->addWidget(extendLabel);
	extendLayout->addWidget(extendBox);
	vLayout->addLayout(extendLayout);

	QPushButton* okButton = new QPushButton(tr("Ok"), this);
	okButton->setDefault(true);
	connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
	QPushButton* cancelButton = new QPushButton(tr("Cancel"), this);
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	QHBoxLayout* dlgButtonsLayout = new QHBoxLayout;
	dlgButtonsLayout->addStretch();
	dlgButtonsLayout->addWidget(okButton);
	dlgButtonsLayout->addWidget(cancelButton);
	vLayout->addLayout(dlgButtonsLayout);

	setLayout(vLayout);
	setMaximumHeight(sizeHint().height());
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	installEventFilter(this);

	setSelected(-1);
}

Gradient EditGradientDialog::getGradient()
{
	Gradient grad;
	grad.setExtend(extend);
	grad.setStops(stops);
	return grad;
}

void EditGradientDialog::setSelected(int sel)
{
	if(selected == sel)
		return;

	selected = sel;
	bool enabled = (sel >= 0);
	removeButton->setEnabled(enabled);
	colorButton->setEnabled(enabled);
	posEdit->setEnabled(enabled);
	colorPreview->setEnabled(enabled);

	if(enabled)
	{
		auto stop = stops[sel];
		posEdit->setText(QString::number(stop.first));
		colorPreview->setColor(toQColor(stop.second));
	}
	else
		posEdit->setText(QString());
}

void EditGradientDialog::changeExtend(int ext)
{
	extend = ext;
}

void EditGradientDialog::chooseColor()
{
	if(selected < 0 || selected >= static_cast<int>(stops.size()))
		return;

	QColor color = QColorDialog::getColor(toQColor(stops[selected].second), this, "", QColorDialog::ShowAlphaChannel);
	if(color.isValid())
	{
		stops[selected].second = fromQColor(color);
		colorPreview->setColor(color);
		view->update();
	}
}

void EditGradientDialog::changePosition()
{
	if (selected < 0 || selected >= static_cast<int>(stops.size()))
		return;

	bool ok;
#ifdef PANDA_DOUBLE
	float pos = posEdit->text().toDouble(&ok);
#else
	float pos = posEdit->text().toFloat(&ok);
#endif
	if(ok)
	{
		stops[selected].first = pos;
		view->update();
	}
}

void EditGradientDialog::addStop()
{
	Gradient::GradientStop stop;
	stop.first = 0.5;

	int i = stops.size();
	stops.push_back(stop);
	setSelected(i);
	view->setSelected(i);
	view->update();
}

void EditGradientDialog::removeStop()
{
	if (selected < 0 || selected >= static_cast<int>(stops.size()))
		return;

	stops.erase(stops.begin() + selected);

	setSelected(-1);
	view->setSelected(-1);
	view->update();
}

void EditGradientDialog::positionHasChanged()
{
	if (selected < 0 || selected >= static_cast<int>(stops.size()))
		return;

	posEdit->setText(QString::number(stops[selected].first));
}

// Just so that we can use "Enter" to finish the editing of the position,
//  without closing the dialog.
bool EditGradientDialog::eventFilter(QObject *obj, QEvent* event)
{
	if (event->type() == QEvent::KeyPress)
	{
		if(posEdit->hasFocus())
		{
			QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
			if(keyEvent->key() == Qt::Key_Return
				|| keyEvent->key() == Qt::Key_Enter
				|| keyEvent->key() == Qt::Key_Escape)
			{
				posEdit->clearFocus();
				return true;
			}
		}
	}

	return QObject::eventFilter(obj, event);
}

//****************************************************************************//

template<>
class DataWidgetContainer< Gradient > : public BaseOpenDialogObject, public ObjectWithPreview
{
public:
	typedef Gradient value_type;
	QWidget* container;
	PreviewView* preview;
	Gradient theGradient;

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
		if(theGradient != v)
		{
			theGradient = v;
			updatePreview();
		}
	}
	void writeToData(value_type& v)
	{
		if(v != theGradient)
		{
			v = theGradient;
			updatePreview();
		}
	}
	virtual void onShowDialog()
	{
		EditGradientDialog dlg(theGradient, container);
		if(dlg.exec() == QDialog::Accepted)
		{
			theGradient = dlg.getGradient();
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

		QLinearGradient grad(0, 0, size.width(), 0);
		for(const auto&s : theGradient.getStops())
			grad.setColorAt(s.first, toQColor(s.second));
		painter.fillRect(0, 0, size.width(), size.height(), QBrush(grad));
	}
};

//****************************************************************************//

RegisterWidget<SimpleDataWidget<Gradient> > DWClass_gradient("default");
RegisterWidget<OpenDialogDataWidget<std::vector<Gradient>, ListDataWidgetDialog<std::vector<Gradient> > > > DWClass_gradients_list("generic");
RegisterWidget<OpenDialogDataWidget<Animation<Gradient>, AnimationDataWidgetDialog<Animation<Gradient> > > > DWClass_gradients_animation("default");
