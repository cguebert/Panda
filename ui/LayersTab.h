#ifndef LAYERSTAB_H
#define LAYERSTAB_H

#include <QWidget>
#include <QStyledItemDelegate>

class QTableWidget;
class QTableWidgetItem;
class QComboBox;
class QSlider;
class QPushButton;
class QLineEdit;

namespace panda
{
	class PandaDocument;
	class PandaObject;
	class Layer;
}

class LayersTab : public QWidget
{
	Q_OBJECT
public:
	explicit LayersTab(panda::PandaDocument* document, QWidget *parent = 0);

protected:
	QLineEdit* nameEdit;
	QComboBox* compositionBox;
	QSlider* opacitySlider;
	QTableWidget* tableWidget;
	QPushButton *moveUpButton, *moveDownButton;
	panda::PandaDocument* document;
	QList<panda::Layer*> layers;
	panda::Layer* selectedLayer;
	
signals:
	
public slots:
	void updateTable();
	void modifiedDocument();
	void addedObject(panda::PandaObject*);
	void removedObject(panda::PandaObject*);
	void itemClicked(QTableWidgetItem*);
	void nameChanged(QString);
	void compositionModeChanged(int);
	void opacityChanged(int);
	void moveLayerUp();
	void moveLayerDown();
};

#endif // LAYERSTAB_H
