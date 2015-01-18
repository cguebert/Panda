#ifndef PANDAOBJECT_H
#define PANDAOBJECT_H

#include <panda/helper/system/config.h>
#include <panda/Data.h>
#include <panda/BaseClass.h>

#include <QSharedPointer>

namespace panda
{

class PandaDocument;

class PANDA_CORE_API PandaObject : public DataNode
{
public:
	PANDA_CLASS(PandaObject, DataNode)
	explicit PandaObject(PandaDocument* document);

	QString getTypeName() const;
	QString getClassName() const;
	QString getNamespaceName() const;
	QString getTemplateName() const;

	QString getName() const;
	quint32 getIndex() const;

	void addData(BaseData* data);
	void removeData(BaseData* data);

	void addOutput(BaseData& data);
	using DataNode::addOutput;

	BaseData* getData(const QString& name) const;
	QList<BaseData*> getDatas() const;
	QList<BaseData*> getInputDatas() const;
	QList<BaseData*> getOutputDatas() const;

	template<class TValue>
	BaseData::BaseInitData initData(Data<TValue>* data, QString name, QString help)
	{
		BaseData::BaseInitData init;
		init.data = data;
		init.name = name;
		init.help = help;
		init.owner = this;
		return init;
	}

	template<class TValue>
	typename Data<TValue>::InitData initData(Data<TValue>* data, const TValue& value, QString name, QString help)
	{
		typename Data<TValue>::InitData init;
		init.data = data;
		init.name = name;
		init.help = help;
		init.owner = this;
		init.value = value;
		return init;
	}

	virtual void postCreate();
	virtual void preDestruction();
	virtual void reset() {}
	virtual void update();
	virtual void updateIfDirty() const;
	virtual void setDirtyValue(const DataNode* caller);
	virtual void beginStep();
	virtual void endStep();

	virtual void save(QDomDocument& doc, QDomElement& elem, const QList<PandaObject*> *selected = nullptr);
	virtual void load(QDomElement &elem);

	virtual void dataSetParent(BaseData* data, BaseData* parent);

	void emitModified();
	void emitDirty();

	bool doesLaterUpdate();

	PandaDocument* getParentDocument();

	virtual void addedToDocument() {}		/// The object is being added
	virtual void removedFromDocument() {}	/// Ths object is being removed (but not deleted as it can be undone later)

	bool isUpdating(); /// True during the execution of update()
	void setInStep(bool inStep); /// The document will force the value of the flag even before calling beginStep

protected:
	PandaDocument* m_parentDocument;
	quint32 m_index;
	bool m_doEmitModified, m_doEmitDirty, m_isInStep;
	bool m_laterUpdate; // Flag for the scheduler: the outputs will be dirty later in the timestep (maybe multiple times)
	mutable bool m_isUpdating;
	bool m_destructing;
	QString m_name;
	QList<BaseData*> m_datas;

	void setInternalData(const QString& name, quint32 index);
	friend class ObjectFactory;
};

//****************************************************************************//

inline QString PandaObject::getTypeName() const
{ return getClass()->getTypeName(); }

inline QString PandaObject::getClassName() const
{ return getClass()->getClassName(); }

inline QString PandaObject::getNamespaceName() const
{ return getClass()->getNamespaceName(); }

inline QString PandaObject::getTemplateName() const
{ return getClass()->getTemplateName(); }

inline QString PandaObject::getName() const
{ return m_name; }

inline quint32 PandaObject::getIndex() const
{ return m_index; }

inline void PandaObject::beginStep()
{ m_isInStep = true; }

inline void PandaObject::endStep()
{ m_isInStep = false; }

inline QList<BaseData*> PandaObject::getDatas() const
{ return m_datas; }

inline void PandaObject::setInternalData(const QString& name, quint32 index)
{ m_name = name; m_index = index; }

inline bool PandaObject::doesLaterUpdate()
{ return m_laterUpdate; }

inline PandaDocument* PandaObject::getParentDocument()
{ return m_parentDocument; }

inline bool PandaObject::isUpdating()
{ return m_isUpdating; }

inline void PandaObject::setInStep(bool inStep)
{ m_isInStep = inStep; }

} // namespace Panda

#endif // PANDAOBJECT_H
