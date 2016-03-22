#ifndef LAYERSTAB_H
#define LAYERSTAB_H

#include <QWidget>
#include <QStyledItemDelegate>

#include <panda/messaging.h>

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
	class BaseLayer;
}

class LayersTab : public QWidget
{
	Q_OBJECT
public:
	explicit LayersTab(panda::PandaDocument* document, QWidget* parent = nullptr);

protected:
	void updateWidgets(panda::BaseLayer* layer, int row = 0);

	QLineEdit* m_nameEdit;
	QComboBox* m_compositionBox;
	QSlider* m_opacitySlider;
	QTableWidget* m_tableWidget;
	QPushButton *m_moveUpButton, *m_moveDownButton;

	panda::PandaDocument* m_document;
	QList<panda::BaseLayer*> m_layers;
	panda::BaseLayer* m_selectedLayer;

	panda::msg::Observer m_observer;

public slots:
	void updateTable();
	void addedObject(panda::PandaObject*);
	void removedObject(panda::PandaObject*);
	void dirtyObject(panda::PandaObject*);
	void modifiedObject(panda::PandaObject*);
	void itemClicked(QTableWidgetItem*);
	void nameChanged();
	void compositionModeChanged(int);
	void opacityChanged(int);
	void moveLayerUp();
	void moveLayerDown();
	void reorderObjects();
};

#endif // LAYERSTAB_H
