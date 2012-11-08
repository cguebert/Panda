#ifndef LAYERSTAB_H
#define LAYERSTAB_H

#include <QWidget>
#include <QStyledItemDelegate>

class QTableWidget;
class QComboBox;
class QSlider;

namespace panda
{
	class PandaDocument;
}

class LayersTab : public QWidget
{
	Q_OBJECT
public:
	explicit LayersTab(panda::PandaDocument* document, QWidget *parent = 0);

protected:
	QTableWidget* tableWidget;
	QComboBox* compositionBox;
	QSlider* opacitySlider;
	panda::PandaDocument* document;
	
signals:
	
public slots:
	void updateTable();
};

#endif // LAYERSTAB_H
