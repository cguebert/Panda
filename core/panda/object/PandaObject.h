#ifndef PANDAOBJECT_H
#define PANDAOBJECT_H

#include <panda/helper/system/config.h>
#include <panda/data/BaseClass.h>
#include <panda/data/Data.h>

#include <memory>
#include <vector>

namespace panda
{

class PandaDocument;
class XmlElement;

class PANDA_CORE_API PandaObject : public DataNode
{
public:
	PANDA_CLASS(PandaObject, DataNode)
	explicit PandaObject(PandaDocument* document);

	const std::string& getName() const;
	uint32_t getIndex() const;

	void addData(BaseData* data, int index = -1); // Insert a new Data at the specified index. If index < 0, add at the end
	void removeData(BaseData* data);

	void addOutput(BaseData& data);
	using DataNode::addOutput;

	BaseData* getData(const std::string& name) const;
	const std::vector<BaseData*>& getDatas() const;
	std::vector<BaseData*> getInputDatas() const;
	std::vector<BaseData*> getOutputDatas() const;

	BaseData::BaseInitData initData(std::string name, std::string help);

	template<class ValueType>
	typename BaseData::InitData<ValueType> initData(const ValueType& value, std::string name, std::string help)
	{ return BaseData::InitData<ValueType>(value, name, help, this); }

	virtual void postCreate();
	virtual void preDestruction();
	virtual void reset() {}
	virtual void update();
	virtual void updateIfDirty() const;
	virtual void setDirtyValue(const DataNode* caller);
	virtual void beginStep();
	virtual void endStep();

	virtual void save(XmlElement& elem, const std::vector<PandaObject*> *selected = nullptr);
	virtual void load(XmlElement& elem);

	virtual void dataSetParent(BaseData* data, BaseData* parent);

	void emitModified();
	void emitDirty();

	bool doesLaterUpdate();
	bool updateOnMainThread();

	PandaDocument* getParentDocument();

	virtual void addedToDocument() {}		/// The object is being added
	virtual void removedFromDocument() {}	/// Ths object is being removed (but not deleted as it can be undone later)

	bool isUpdating(); /// True during the execution of update()
	void setInStep(bool inStep); /// The document will force the value of the flag even before calling beginStep

protected:
	PandaDocument* m_parentDocument;
	uint32_t m_index = 0;
	bool m_doEmitModified = true, m_doEmitDirty = true, m_isInStep = false;
	bool m_laterUpdate = false; // Flag for the scheduler: the outputs will be dirty later in the timestep (maybe multiple times)
	bool m_updateOnMainThread = false; // Flag for the scheduler: if true, this object will always be updated on the main thread
	mutable bool m_isUpdating = false;
	bool m_destructing = false;
	std::string m_name;
	std::vector<BaseData*> m_datas;

	void setInternalData(const std::string& name, uint32_t index);
	friend class ObjectFactory;
};

//****************************************************************************//

inline PandaObject::PandaObject(PandaDocument* document)
	: m_parentDocument(document) { }

inline BaseData::BaseInitData PandaObject::initData(std::string name, std::string help)
{ return BaseData::BaseInitData(name, help, this); }

inline const std::string& PandaObject::getName() const
{ return m_name; }

inline uint32_t PandaObject::getIndex() const
{ return m_index; }

inline void PandaObject::beginStep()
{ m_isInStep = true; }

inline void PandaObject::endStep()
{ m_isInStep = false; }

inline const std::vector<BaseData*>& PandaObject::getDatas() const
{ return m_datas; }

inline void PandaObject::setInternalData(const std::string& name, uint32_t index)
{ m_name = name; m_index = index; }

inline bool PandaObject::doesLaterUpdate()
{ return m_laterUpdate; }

inline bool PandaObject::updateOnMainThread()
{ return m_updateOnMainThread; }

inline PandaDocument* PandaObject::getParentDocument()
{ return m_parentDocument; }

inline bool PandaObject::isUpdating()
{ return m_isUpdating; }

inline void PandaObject::setInStep(bool inStep)
{ m_isInStep = inStep; }

} // namespace Panda

#endif // PANDAOBJECT_H
