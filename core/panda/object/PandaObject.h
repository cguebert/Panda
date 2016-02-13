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

	const std::string& getName() const; /// Returns the name of the object (what is shown in the graph view)
	uint32_t getIndex() const; /// Returns the index of creation of this object (will not change during the life of the document)

	void addData(BaseData* data, int index = -1); /// Insert a new Data at the specified index. If index < 0, add at the end
	void removeData(BaseData* data); /// Remove the data from the list (it will not be shown in the GUI nor serialized)

	void addOutput(BaseData& data); /// data will be set as read-only before being added
	using DataNode::addOutput;

	BaseData* getData(const std::string& name) const; /// Returns the Data with the given name, or null
	const std::vector<BaseData*>& getDatas() const; /// Access to the list of Datas
	std::vector<BaseData*> getInputDatas() const; /// Get only the Datas registered as input to this object
	std::vector<BaseData*> getOutputDatas() const; /// Get only the Datas registered as output to this object

	/// Helper function to set the owner of a Data as this object, which will automatically add the Data to the list
	BaseData::BaseInitData initData(std::string name, std::string help);

	/// Templated version of initData which accepts an initial value (otherwise the value is default constructed)
	template<class ValueType>
	typename BaseData::InitData<ValueType> initData(const ValueType& value, std::string name, std::string help)
	{ return BaseData::InitData<ValueType>(value, name, help, this); }

	virtual void postCreate(); /// Called by the factory after the name and index have been set
	virtual void preDestruction(); /// Called just before this object is freed
	virtual void reset() {} /// Called when rewinding the document
	virtual void update() override; /// Do the computations in this method
	virtual void updateIfDirty() const override; /// This adds logging before calling update
	virtual void setDirtyValue(const DataNode* caller) override; /// Only adds logging on top of DataNode::setDirtyValue
	virtual void beginStep(); /// Called at the beginning of each step (update can be called without this being called if outside of animation)
	virtual void endStep(); /// Called at the end of each step

	virtual void save(XmlElement& elem, const std::vector<PandaObject*> *selected = nullptr); /// Save the object in XML
	virtual void load(XmlElement& elem); /// Load the object from XML

	virtual void dataSetParent(BaseData* data, BaseData* parent); /// Changed the parent of the data, and emit the modified signal

	void emitModified(); /// Emit the modified signal (unless special cases, like if the object is being destroyed)
	void emitDirty(); /// Emit the dirty signal  (unless special cases)

	bool doesLaterUpdate(); /// Access to the read-only laterUpdate property
	bool updateOnMainThread(); /// Access to the read-only updateOnMainThread property

	PandaDocument* getParentDocument(); /// Get the parent document of this object (for a document, this is itself)

	virtual void addedToDocument() {}		/// The object is being added
	virtual void removedFromDocument() {}	/// Ths object is being removed (but not deleted as it can be undone later)

	bool isUpdating(); /// True during the execution of update()
	void setInStep(bool inStep); /// The document will force the value of the flag even before calling beginStep

protected:
	void setInternalData(const std::string& name, uint32_t index); /// Should only be called by the Object Factory, to set the object's name and index
	friend class ObjectFactory;

	PandaDocument* m_parentDocument = nullptr;
	std::vector<BaseData*> m_datas;
	uint32_t m_index = 0;
	std::string m_name;

	bool m_doEmitModified = true;
	bool m_doEmitDirty = true;
	bool m_isInStep = false;
	bool m_laterUpdate = false; // Flag for the scheduler: the outputs will be dirty later in the timestep (maybe multiple times)
	bool m_updateOnMainThread = false; // Flag for the scheduler: if true, this object will always be updated on the main thread
	mutable bool m_isUpdating = false; // Mutable as it will modified in const methods
	bool m_destructing = false;
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
