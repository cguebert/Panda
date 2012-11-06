#include "Group.h"

#include <ui/GraphView.h>
#include <panda/PandaDocument.h>
#include <panda/ObjectFactory.h>
#include <panda/Layer.h>
#include <panda/Renderer.h>

#include <QMessageBox>
#include <QPainter>

namespace panda
{

Group::Group(PandaDocument *parent)
    : PandaObject(parent)
    , groupName(initData(&groupName, QString("Group"), "name", "Name to be displayed for this group"))
{
}

Group::~Group()
{
}

bool Group::createGroup(PandaDocument* doc, GraphView* view)
{
    if(doc->getNbSelected() < 2)
        return false;

	PandaDocument::ObjectsIterator iter = doc->getSelectionIterator();

	// Verify that all selected renderers are in the same layer
	Layer* layer = NULL;
	while(iter.hasNext())
	{
		PandaObject* object = iter.next();
		Renderer* renderer = dynamic_cast<Renderer*>(object);
		if(renderer)
		{
			if(!layer)
				layer = dynamic_cast<Layer*>(renderer->getParentDock());
			else if(layer != renderer->getParentDock())
			{
				QMessageBox::warning(NULL, tr("Panda"),
					tr("All renderers must be placed in the same layer."));
				return false;
			}
		}
	}

	Group* group = dynamic_cast<Group*>(doc->createObject(ObjectFactory::getClassName<Group>()));
    if(!group)
        return false;

    // Find center of the selection
    QRectF totalView;
	iter.toFront();
    while(iter.hasNext())
    {
        PandaObject* object = iter.next();
        QRectF objectArea = view->getObjectDrawStruct(object)->getObjectArea();
        totalView = totalView.united(objectArea);
    }

    // Put the new object there
    ObjectDrawStruct* ods = view->getObjectDrawStruct(group);
    QSize objSize = ods->getObjectSize() / 2;
    ods->move(totalView.center() - view->getViewDelta() - ods->getPosition() - QPointF(objSize.width(), objSize.height()));
    QPointF groupPos = ods->getPosition();

	// If multiple outside datas are connected to the same data, merge them
	QMap<BaseData*, BaseData*> connectedInputDatas, connectedOutputDatas;

    // Adding the objects
    iter.toFront();
    while(iter.hasNext())
    {
        PandaObject* object = iter.next();
        group->objects.append(object);
        object->setParent(group);

        // Storing the position of this object in respect to the group object
        QPointF delta = view->getObjectDrawStruct(object)->getPosition() - groupPos;
        group->positions[object] = delta;

        // Adding input datas
        foreach(BaseData* data, object->getInputDatas())
        {
            BaseData* otherData = data->getParent();
            if(otherData)
            {
                PandaObject* connected = otherData->getOwner();
                if(connected && !doc->isSelected(connected))
				{
					BaseData* createdData = NULL;
					if(!connectedInputDatas.contains(otherData))
					{
						createdData = group->duplicateData(data);
						createdData->copyValueFrom(otherData);
						group->addInput(createdData);
						group->dataSetParent(createdData, otherData);
						connectedInputDatas.insert(otherData, createdData);
					}
					else
					{
						createdData = connectedInputDatas.value(otherData);
						QString name = group->findAvailableDataName(otherData->getName(), createdData);
						if(name != createdData->getName())
						{
							createdData->setName(name);
							createdData->setHelp(otherData->getHelp());
						}
					}

					if(createdData)
						data->getOwner()->dataSetParent(data, createdData);
				}
            }
        }

        // Adding ouput datas
        foreach(BaseData* data, object->getOutputDatas())
        {
            foreach(DataNode* otherNode, data->getOutputs())
            {
                BaseData* otherData = dynamic_cast<BaseData*>(otherNode);
                if(otherData)
                {
                    PandaObject* connected = otherData->getOwner();
                    if(connected && !doc->isSelected(connected))
					{
						BaseData* createdData = NULL;
						if(!connectedOutputDatas.contains(data))
						{
							createdData = group->duplicateData(data);
							createdData->copyValueFrom(data);
							group->dataSetParent(createdData, data);
							group->addOutput(createdData);
							connectedOutputDatas.insert(data, createdData);
						}
						else
							createdData = connectedOutputDatas.value(data);

						if(createdData)
							otherData->getOwner()->dataSetParent(otherData, createdData);
					}
                }
            }
        }
    }

    // Select the group
    doc->selectNone();
    doc->setCurrentSelectedObject(group);

    // Removing the objects from the document
    iter.toFront();
    while(iter.hasNext())
        doc->doRemoveObject(iter.next(), false); // Do not delete it

    view->modifiedObject(group);
    view->updateLinkTags();

    return true;
}

bool Group::ungroupSelection(PandaDocument* doc, GraphView* view)
{
    if(doc->getNbSelected() < 1)
        return false;

    QList<Group*> groups;
    PandaDocument::ObjectsIterator objIter = doc->getSelectionIterator();
    while(objIter.hasNext())
    {
		Group* group = dynamic_cast<Group*>(objIter.next());
        if(group)
            groups.append(group);
    }

    if(groups.isEmpty())
        return false;

    doc->selectNone();

    // For each group in the selection
    QListIterator<Group*> iter(groups);
    while(iter.hasNext())
    {
        Group* group = iter.next();
        QPointF groupPos = view->getObjectDrawStruct(group)->getPosition();

        // Putting the objects back into the document
        objIter = group->objects;
        while(objIter.hasNext())
        {
            PandaObject* object = objIter.next();
            doc->doAddObject(object);
            doc->selectionAdd(object);
            object->setParent(doc);

            // Placing the object in the view
            ObjectDrawStruct* ods = view->getObjectDrawStruct(object);
            ods->move(groupPos + group->positions[object] - ods->getPosition());
        }

        // Reconnecting datas
        foreach(QSharedPointer<BaseData> data, group->groupDatas)
        {
            BaseData* parent = data->getParent();
            foreach(DataNode* node, data->getOutputs())
            {
                BaseData* outData = dynamic_cast<BaseData*>(node);
                if(outData)
                    outData->getOwner()->dataSetParent(outData, parent);
            }
        }

        doc->doRemoveObject(group);
    }

    view->updateLinkTags();

    return true;
}

void Group::save(QDataStream& out)
{
    // Saving group datas
    out << (quint32)groupDatas.size();
    foreach(QSharedPointer<BaseData> data, groupDatas)
        out << (quint32)data->getFullType()
            << data->isInput() << data->isOutput()
            << data->getName() << data->getHelp();

    // Saving data values
    PandaObject::save(out);

    typedef QPair<BaseData*, BaseData*> DataPair;
    QList<DataPair> links;

    typedef QPair<quint32, quint32> IntPair;
	QList<IntPair> dockedObjects;

    // Saving objects in this group
    ObjectFactory* factory = ObjectFactory::getInstance();
    out << (quint32)objects.size();
    QList<PandaObject*>::iterator iter;
    foreach(PandaObject* object, objects)
    {
        out << factory->getRegistryName(object);
        out << object->getIndex();

        object->save(out);

        out << positions[object];

        // Preparing links
        foreach(BaseData* data, object->getInputDatas())
        {
            BaseData* parent = data->getParent();
            if(parent)
                links.append(qMakePair(data, parent));
        }

		// Preparing dockables list for docks
		DockObject* dock = dynamic_cast<DockObject*>(object);
		if(dock)
		{
			DockObject::DockablesIterator dockableIter = dock->getDockablesIterator();
			while(dockableIter.hasNext())
				dockedObjects.append(qMakePair(dock->getIndex(), dockableIter.next()->getIndex()));
		}
    }

    // Links from objects to the output of the group
    foreach(BaseData* data, getOutputDatas())
    {
        BaseData* parent = data->getParent();
        if(parent)
            links.append(qMakePair(data, parent));
    }

    // Saving links
    out << (quint32)links.size();
    foreach(DataPair link, links)
    {
        if(link.first->getOwner() == this)
            out << (quint32)0;
        else
            out << link.first->getOwner()->getIndex();
        out << link.first->getName();

        if(link.second->getOwner() == this)
            out << (quint32)0;
        else
            out << link.second->getOwner()->getIndex();
        out << link.second->getName();
    }

	// Saving docked objects list
	out << (quint32)dockedObjects.size();
	foreach(IntPair dockable, dockedObjects)
		out << dockable.first << dockable.second;
}

void Group::save(QTextStream& out)
{
    // Saving group datas
    out << (quint32)groupDatas.size() << endl;
    foreach(QSharedPointer<BaseData> data, groupDatas)
        out << (quint32)data->getFullType() << " "
            << data->isInput() << " " << data->isOutput() << endl
            << data->getName() << endl << data->getHelp() << endl;

    // Saving data values
    PandaObject::save(out);

    typedef QPair<BaseData*, BaseData*> DataPair;
    QList<DataPair> links;

    typedef QPair<quint32, quint32> IntPair;
	QList<IntPair> dockedObjects;

    // Saving objects in this group
    ObjectFactory* factory = ObjectFactory::getInstance();
    out << (quint32)objects.size() << endl;
    QList<PandaObject*>::iterator iter;
    foreach(PandaObject* object, objects)
    {
        out << factory->getRegistryName(object) << endl;
        out << object->getIndex() << endl;

        object->save(out);

        QPointF pos = positions[object];
        out << pos.x() << " " << pos.y() << endl;

        // Preparing links
        foreach(BaseData* data, object->getInputDatas())
        {
            BaseData* parent = data->getParent();
            if(parent)
                links.append(qMakePair(data, parent));
        }

		// Preparing dockables list for docks
		DockObject* dock = dynamic_cast<DockObject*>(object);
		if(dock)
		{
			DockObject::DockablesIterator dockableIter = dock->getDockablesIterator();
			while(dockableIter.hasNext())
				dockedObjects.append(qMakePair(dock->getIndex(), dockableIter.next()->getIndex()));
		}
    }

    // Links from objects to the output of the group
    foreach(BaseData* data, getOutputDatas())
    {
        BaseData* parent = data->getParent();
        if(parent)
            links.append(qMakePair(data, parent));
    }

    // Saving links
    out << (quint32)links.size() << endl;
    foreach(DataPair link, links)
    {
        if(link.first->getOwner() == this)
            out << (quint32)0;
        else
            out << link.first->getOwner()->getIndex();
        out << " " << link.first->getName() << endl;

        if(link.second->getOwner() == this)
            out << (quint32)0;
        else
            out << link.second->getOwner()->getIndex();
        out << " " << link.second->getName() << endl;
    }

	// Saving docked objects list
	out << (quint32)dockedObjects.size() << endl;
	foreach(IntPair dockable, dockedObjects)
		out << dockable.first << " " << dockable.second << endl;
}

void Group::load(QDataStream& in)
{
    quint32 nbDatas;
    in >> nbDatas;
    for(quint32 i=0; i<nbDatas; ++i)
    {
        quint32 type;
        bool input, output;
        QString name, help;
        in >> type >> input >> output
              >> name >> help;
        BaseData* data = createDataFromFullType(type, name, help, this);
        groupDatas.append( QSharedPointer<BaseData>(data) );
        if(input)
            addInput(data);
        if(output)
            addOutput(data);
    }

    // Loading data values
    PandaObject::load(in);

    QMap<quint32, PandaObject*> importObjectsMap;
    ObjectFactory* factory = ObjectFactory::getInstance();
	PandaDocument* doc = dynamic_cast<PandaDocument*>(parent());

    quint32 nbObjects;
    in >> nbObjects;
    for(quint32 i=0; i<nbObjects; ++i)
    {
        QString registryName;
        quint32 index;
        in >> registryName >> index;

        PandaObject* object = factory->create(registryName, doc);
        if(object)
        {
            importObjectsMap[index] = object;
            objects.append(object);
            object->setParent(this);

            object->load(in);

            QPointF delta;
            in >> delta;
            positions[object] = delta;
        }
        else
        {
            QMessageBox::warning(NULL, tr("Panda"),
                tr("Could not create the object %1.\nA plugin must be missing.")
                .arg(registryName));
            return;
        }
    }

    // Create links
    quint32 nbLinks;
    in >> nbLinks;
    for(quint32 i=0; i<nbLinks; ++i)
    {
        quint32 index1, index2;
        QString name1, name2;
        in >> index1 >> name1 >> index2 >> name2;

        PandaObject *object1, *object2;

        BaseData *data1=NULL, *data2=NULL;
        if(index1)
        {
            object1 = importObjectsMap[index1];
            if(object1)
                data1 = object1->getData(name1);
        }
        else
            data1 = getData(name1);

        if(index2)
        {
            object2 = importObjectsMap[index2];
            if(object2)
                data2 = object2->getData(name2);
        }
        else
            data2 = getData(name2);

        if(data1 && data2)
            data1->setParent(data2);
    }

	// Put dockables in their docks
	quint32 nbDockedObjects;
	in >> nbDockedObjects;
	for(quint32 i=0; i<nbDockedObjects; ++i)
	{
		quint32 dockIndex, dockableIndex;
		in >> dockIndex >> dockableIndex;

		DockObject* dock = dynamic_cast<DockObject*>(importObjectsMap[dockIndex]);
		DockableObject* dockable = dynamic_cast<DockableObject*>(importObjectsMap[dockableIndex]);
		if(dock && dockable)
		{
			DockObject* defaultDock = dockable->getDefaultDock(doc);
			if(defaultDock)
				defaultDock->removeDockable(dockable);
			dock->addDockable(dockable);
		}
	}

    emit modified(this);
}

void Group::load(QTextStream& in)
{
    quint32 nbDatas;
    in >> nbDatas;
    for(quint32 i=0; i<nbDatas; ++i)
    {
        quint32 type, input, output;
        QString name, help;
        in >> type >> input >> output;
        in.skipWhiteSpace();
        name = in.readLine();
        in.skipWhiteSpace();
        help = in.readLine();

        BaseData* data = createDataFromFullType(type, name, help, this);
        groupDatas.append( QSharedPointer<BaseData>(data) );
        if(input)
            addInput(data);
        if(output)
            addOutput(data);
    }

    // Loading data values
    PandaObject::load(in);

    QMap<quint32, PandaObject*> importObjectsMap;
    ObjectFactory* factory = ObjectFactory::getInstance();
	PandaDocument* doc = dynamic_cast<PandaDocument*>(parent());

    quint32 nbObjects;
    in >> nbObjects;
    for(quint32 i=0; i<nbObjects; ++i)
    {
        QString registryName;
        quint32 index;
        in.skipWhiteSpace();
        registryName = in.readLine();
        in >> index;

        PandaObject* object = factory->create(registryName, doc);
        if(object)
        {
            importObjectsMap[index] = object;
            objects.append(object);
            object->setParent(this);

            object->load(in);

            qreal x, y;
            in >> x >> y;
            positions[object] = QPointF(x, y);
        }
        else
        {
            QMessageBox::warning(NULL, tr("Panda"),
                tr("Could not create the object %1.\nA plugin must be missing.")
                .arg(registryName));
            return;
        }
    }

    // Create links
    quint32 nbLinks;
    in >> nbLinks;
    for(quint32 i=0; i<nbLinks; ++i)
    {
        quint32 index1, index2;
        QString name1, name2;
        in >> index1;
        in.skipWhiteSpace();
        name1 = in.readLine();
        in >> index2;
        in.skipWhiteSpace();
        name2 = in.readLine();

        PandaObject *object1, *object2;

        BaseData *data1=NULL, *data2=NULL;
        if(index1)
        {
            object1 = importObjectsMap[index1];
            if(object1)
                data1 = object1->getData(name1);
        }
        else
            data1 = getData(name1);

        if(index2)
        {
            object2 = importObjectsMap[index2];
            if(object2)
                data2 = object2->getData(name2);
        }
        else
            data2 = getData(name2);

        if(data1 && data2)
            data1->setParent(data2);
    }

	// Put dockables in their docks
	quint32 nbDockedObjects;
	in >> nbDockedObjects;
	for(quint32 i=0; i<nbDockedObjects; ++i)
	{
		quint32 dockIndex, dockableIndex;
		in >> dockIndex >> dockableIndex;

		DockObject* dock = dynamic_cast<DockObject*>(importObjectsMap[dockIndex]);
		DockableObject* dockable = dynamic_cast<DockableObject*>(importObjectsMap[dockableIndex]);
		if(dock && dockable)
		{
			DockObject* defaultDock = dockable->getDefaultDock(doc);
			if(defaultDock)
				defaultDock->removeDockable(dockable);
			dock->addDockable(dockable);
		}
	}

    emit modified(this);
}

QString Group::findAvailableDataName(QString baseName, BaseData *data)
{
	QString name = baseName;
	BaseData* testData = getData(name);
	if(testData && testData != data)
	{
		int i=2;
		testData = getData(name + QString::number(i));
		while(testData && testData != data)
		{
			++i;
			testData = getData(name + QString::number(i));
		}
		name = name + QString::number(i);
	}
	return name;
}

BaseData* Group::duplicateData(BaseData* data)
{
    if(!data)
        return NULL;

	QString name = findAvailableDataName(data->getName());

    BaseData* newData = NULL;
    if(data->isSingleValue())
        newData = createDataFromType(data->getValueType(), name, data->getHelp(), this);
    else if(data->isVector())
        newData = createVectorDataFromType(data->getValueType(), name, data->getHelp(), this);
    else if(data->isAnimation())
        newData = createAnimationDataFromType(data->getValueType(), name, data->getHelp(), this);

    newData->setDisplayed(data->isDisplayed());
    newData->setPersistent(data->isPersistent());
    groupDatas.append( QSharedPointer<BaseData>(newData) );

    return newData;
}

int GroupClass = RegisterObject("Group").setClass<Group>().setDescription("Groups many object into a single one").setHidden(true);

} // namespace panda
