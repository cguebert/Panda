#ifndef PANDAOBJECT_H
#define PANDAOBJECT_H

#include <panda/data/BaseClass.h>
#include <panda/data/Data.h>

#include <memory>
#include <vector>

namespace panda
{

class DirtySignalDisabler;
class ObjectAddons;
class ModifiedSignalDisabler;
class PandaDocument;
class XmlElement;

class PANDA_CORE_API PandaObject : public DataNode
{
public:
	PANDA_CLASS(PandaObject, DataNode)
	explicit PandaObject(PandaDocument* document); /// The document is guaranteed to not change during the life of the object
	virtual ~PandaObject();

	const std::string& getName() const; /// Returns the name of the object (what is shown in the graph view)
	uint32_t getIndex() const; /// Returns the index of creation of this object (will not change during the life of the document)

	void addData(BaseData* data, int index = -1); /// Insert a new Data at the specified index. If index < 0, add at the end
	void removeData(BaseData* data); /// Remove the data from the list (it will not be shown in the GUI nor serialized)

	void addOutput(BaseData& data); /// data will be set as read-only before being added
	using DataNode::addOutput; // Can still use "addOutput(DataNode& node)"

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
	virtual void update() override; /// Do the computations in this method. The dirty flag is automatically cleaned after this method, unless "setStillDirty(true)" is called.
	virtual void updateIfDirty() const override; /// This adds logging before calling update
	virtual void setDirtyValue(const DataNode* caller) override; /// Only adds logging on top of DataNode::setDirtyValue
	virtual void beginStep(); /// Called at the beginning of each step (update can be called without this being called if outside of animation)
	virtual void endStep(); /// Called at the end of each step

	virtual void save(XmlElement& elem, const std::vector<PandaObject*> *selected = nullptr); /// Save the object in XML
	virtual void load(const XmlElement& elem); /// Load the object from XML

	virtual void dataSetParent(BaseData* data, BaseData* parent); /// Changed the parent of the data, and emit the modified signal

	void emitModified(); /// Emit the modified signal (unless special cases, like if the object is being destroyed)
	void emitDirty(); /// Emit the dirty signal  (unless special cases)

	bool doesLaterUpdate() const; /// Access to the read-only laterUpdate property
	bool updateOnMainThread() const; /// Access to the read-only updateOnMainThread property

	PandaDocument* parentDocument() const; /// Get the parent document of this object (for a document, this is itself)

	template <class D>
	D* parent() const; /// Get the parent document of this object, casted to the given type

	virtual void addedToDocument() {}		/// The object is being added
	virtual void removedFromDocument() {}	/// Ths object is being removed (but not deleted as it can be undone later)

	bool isUpdating() const; /// True during the execution of update()
	bool isDestructing() const; /// True after preDestruction() has been called

	void setInStep(bool inStep); /// The document will force the value of the flag even before calling beginStep
	bool isInStep() const; /// This is true if update is called while a step is in progress

	virtual NodesList getNonRecursiveInputs() const; /// Get input nodes, without those that can create loops in the graph (see ListLoop and ListBuffer objects)
	virtual NodesList getNonRecursiveOutputs() const; /// Get output nodes, without those that can create loops in the graph

	virtual std::string getLabel() const; /// If not empty, will be shown in the graph view (with the format "label (name)")

	ObjectAddons& addons() const; /// Get the addons for this object

protected:
	void setInternalData(const std::string& name, uint32_t index); /// Should only be called by the Object Factory, to set the object's name and index
	friend class ObjectFactory;

	friend class DirtySignalDisabler;
	friend class ModifiedSignalDisabler;

	void setParentDocument(PandaDocument* doc); /// Set the parent document. Will do nothing if a document has already been set as this object's parent.

	void setLaterUpdate(bool b = true); /// Tell the scheduler that this object will be dirty later in the timestep (maybe multiple times)
	void setUpdateOnMainThread(bool b = true); /// Tell the scheduler that this object will always be updated on the main thread

private:
	PandaDocument* m_parentDocument = nullptr; // Pointer to the parent document
	std::vector<BaseData*> m_datas; // The list of Datas added to this object (via the use of initData in a Data constructor or with addData)
	uint32_t m_index = 0; // The unique index of this object. This is set automatically by the factory
	std::string m_name; // The class name of this object. This is set automatically by the factory
	std::unique_ptr<ObjectAddons> m_addons; // Addons for this object

	bool m_doEmitModified = true; // If false, prevent the emission of the modified signal
	bool m_doEmitDirty = true; // If false, prevent the emission of the dirty signal
	bool m_isInStep = false; // If true, we are in the execution of PandaDocument::step
	bool m_laterUpdate = false; // Flag for the scheduler: the outputs will be dirty later in the timestep (maybe multiple times)
	bool m_updateOnMainThread = false; // Flag for the scheduler: if true, this object will always be updated on the main thread
	bool m_destructing = false; // If true, do not do any computations as the object will be removed from the document
	mutable bool m_isUpdating = false; // Mutable as it will modified in const methods
};

class DirtySignalDisabler
{
public:
	DirtySignalDisabler(PandaObject* object) : m_object(object)
	{ m_object->m_doEmitDirty = false; }

	~DirtySignalDisabler()
	{ m_object->m_doEmitDirty = true; }

private:
	PandaObject* m_object;
};

class ModifiedSignalDisabler
{
public:
	ModifiedSignalDisabler(PandaObject* object) : m_object(object)
	{ m_object->m_doEmitModified = false; }

	~ModifiedSignalDisabler()
	{ m_object->m_doEmitModified = true; }

private:
	PandaObject* m_object;
};

//****************************************************************************//

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

inline bool PandaObject::doesLaterUpdate() const
{ return m_laterUpdate; }

inline bool PandaObject::updateOnMainThread() const
{ return m_updateOnMainThread; }

inline PandaDocument* PandaObject::parentDocument() const
{ return m_parentDocument; }

inline void PandaObject::setParentDocument(PandaDocument* doc)
{ if (!m_parentDocument) m_parentDocument = doc; }

template <class D>
D* PandaObject::parent() const
{
	static_assert(std::is_base_of<PandaDocument, D>::value, "The argument of PandaObject::parent must inherit from PandaDocument");
	return dynamic_cast<D*>(m_parentDocument);
}

inline bool PandaObject::isUpdating() const
{ return m_isUpdating; }

inline bool PandaObject::isDestructing() const
{ return m_destructing; }

inline void PandaObject::setInStep(bool inStep)
{ m_isInStep = inStep; }

inline bool PandaObject::isInStep() const
{ return m_isInStep; }

inline void PandaObject::setLaterUpdate(bool b)
{ m_laterUpdate = b; }

inline void PandaObject::setUpdateOnMainThread(bool b)
{ m_updateOnMainThread = b; }

inline DataNode::NodesList PandaObject::getNonRecursiveInputs() const
{ return getInputs(); }

inline DataNode::NodesList PandaObject::getNonRecursiveOutputs() const
{ return getOutputs(); }

inline ObjectAddons& PandaObject::addons() const
{ return *m_addons.get(); }

} // namespace Panda

#endif // PANDAOBJECT_H
