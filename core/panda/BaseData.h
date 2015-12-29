#ifndef BASEDATA_H
#define BASEDATA_H

#include <panda/DataNode.h>

#include <QDomDocument>

namespace panda
{

class PandaObject;
class BaseData;
class AbstractDataCopier;
namespace types { class AbstractDataTrait; }

class PANDA_CORE_API VoidDataAccessor
{
public:
	VoidDataAccessor(BaseData* d);
	~VoidDataAccessor();
	void* get();
	operator void*();

protected:
	BaseData* m_data;
	void* m_value;
};

//****************************************************************************//

class PANDA_CORE_API BaseData : public DataNode
{
public:
	PANDA_CLASS(BaseData, DataNode)

	class BaseInitData
	{
	public:
		BaseInitData() : owner(nullptr) {}
		BaseInitData(const std::string& name, const std::string& help, PandaObject* owner)
			: name(name), help(help), owner(owner) {}
		PandaObject* owner;
		std::string name, help;
	};

	template <class ValueType>
	class InitData : public BaseData::BaseInitData
	{
	public:
		InitData() : value(ValueType()) {}
		InitData(const ValueType& value, const std::string& name, const std::string& help, PandaObject* owner)
			: BaseInitData(name, help, owner), value(value) {}
		ValueType value;
	};

	explicit BaseData(const BaseInitData& init, const std::type_info& type);
	BaseData(const std::string& name, const std::string& help, PandaObject* owner, const std::type_info& type);
	virtual ~BaseData() {}

	const std::string& getName() const;	/// Name used in the UI and for saving / loading
	void setName(const std::string& name);
	const std::string& getHelp() const;	/// Message describing the Data
	void setHelp(const std::string& help);
	const std::string& getWidget() const; /// Custom widget to use for this Data
	void setWidget(const std::string& widget);
	const std::string& getWidgetData() const; /// Some custom widgets need parameters, they are saved in string format
	void setWidgetData(const std::string& widgetData);

	bool isSet() const; /// Has the value changed from the default
	void unset();		/// As if the current value is the default
	void forceSet();	/// Consider the current value as changed

	virtual int getCounter() const; /// The counter usually increments at each value change (after disconnecting from a parent, can decrement quite a bit)

	bool isReadOnly() const;	/// Only used in the UI, automatically set if the Data is an input
	void setReadOnly(bool readOnly);
	bool isDisplayed() const;	/// Is it shown in the UI
	void setDisplayed(bool displayed);
	bool isPersistent() const;	/// Is it saved (some types don't want it)
	void setPersistent(bool persistent);
	bool isInput() const;		/// Is it used as of one of the inputs of a PandaObject
	void setInput(bool input);
	bool isOutput() const;		/// Is it used as of one of the outputs of a PandaObject
	void setOutput(bool output);

	PandaObject* getOwner() const; /// The PandaObject that owns this Data
	void setOwner(PandaObject* owner);

	virtual bool validParent(const BaseData* parent) const; /// Can parent be connected to this Data
	virtual void setParent(BaseData* parent); /// Set the other Data as the parent to this one (its value will be copied each time it changes)
	BaseData* getParent() const; /// Returns the current parent, or nullptr

	const types::AbstractDataTrait* getDataTrait() const; /// Return a class describing the type stored in this Data
	virtual const void* getVoidValue() const = 0; /// Return a void* pointing to the value (use the DataTrait to exploit it)
	VoidDataAccessor getVoidAccessor(); /// Return a wrapper around the void*, that will call endEdit when destroyed

	virtual std::string getDescription() const; /// Get a readable name of the type stored in this Data

	void copyValueFrom(const BaseData* parent); /// Copy the value from parent to this Data

	virtual void save(QDomDocument& doc, QDomElement& elem) const; /// Save the value of the Data in a Xml node
	virtual void load(QDomElement& elem); /// Load the value from Xml

	void setDirtyValue(const DataNode* caller) override;

protected:
	virtual void doAddInput(DataNode& node) override;
	virtual void doRemoveInput(DataNode& node) override;
	virtual void doAddOutput(DataNode& node) override;
	virtual void doRemoveOutput(DataNode& node) override;

	friend class VoidDataAccessor;
	virtual void* beginVoidEdit() = 0;
	virtual void endVoidEdit() = 0;

	void initInternals(const std::type_info& type);

	enum DataFlagsEnum
	{
		FLAG_NONE = 0,
		FLAG_READONLY = 1 << 0, /// Can this Data be edited in the GUI
		FLAG_DISPLAYED = 1 << 1, /// Is it displayed in the GUI
		FLAG_PERSISTENT = 1 << 2, /// Is it saved
		FLAG_INPUT = 1 << 3, /// Is it an input of an object
		FLAG_OUTPUT = 1 << 4, /// Is it an output of an object
		FLAG_VALUE_SET = 1 << 5, /// Was the value modified from the default
		FLAG_SETPARENTPROTECTION = 1 << 6 /// (internal) Are we modifying the parentage of this data
	};
	typedef unsigned DataFlags;
	enum { FLAG_DEFAULT = FLAG_DISPLAYED | FLAG_PERSISTENT };

	void setFlag(DataFlagsEnum flag, bool b);
	bool getFlag(DataFlagsEnum flag) const;

	DataFlags m_dataFlags;
	int m_counter;
	PandaObject* m_owner;
	BaseData* m_parentBaseData;
	types::AbstractDataTrait* m_dataTrait;
	AbstractDataCopier* m_dataCopier;
	std::string m_name, m_help, m_widget, m_widgetData;

private:
	BaseData() {}
};

//****************************************************************************//

inline VoidDataAccessor::VoidDataAccessor(BaseData* data)
	: m_data(data), m_value(data->beginVoidEdit()) {}

inline VoidDataAccessor::~VoidDataAccessor()
{ m_data->endVoidEdit(); }

inline void* VoidDataAccessor::get()
{ return m_value; }

inline VoidDataAccessor::operator void *()
{ return m_value; }

inline const std::string& BaseData::getName() const
{ return m_name; }

inline void BaseData::setName(const std::string& name)
{ m_name = name; }

inline const std::string& BaseData::getHelp() const
{ return m_help; }

inline void BaseData::setHelp(const std::string& help)
{ m_help = help; }

inline const std::string& BaseData::getWidget() const
{ return m_widget; }

inline void BaseData::setWidget(const std::string& widget)
{ m_widget = widget; }

inline const std::string& BaseData::getWidgetData() const
{ return m_widgetData; }

inline void BaseData::setWidgetData(const std::string& widgetData)
{ m_widgetData = widgetData; }

inline bool BaseData::isSet() const
{ return getFlag(FLAG_VALUE_SET); }

inline void BaseData::unset()
{ setFlag(FLAG_VALUE_SET, false); }

inline void BaseData::forceSet()
{ setFlag(FLAG_VALUE_SET, true); }

inline int BaseData::getCounter() const
{ if(m_parentBaseData) return m_parentBaseData->getCounter(); return m_counter; }

inline bool BaseData::isReadOnly() const
{ return getFlag(FLAG_READONLY); }

inline void BaseData::setReadOnly(bool readOnly)
{ setFlag(FLAG_READONLY, readOnly); }

inline bool BaseData::isDisplayed() const
{ return getFlag(FLAG_DISPLAYED); }

inline void BaseData::setDisplayed(bool displayed)
{ setFlag(FLAG_DISPLAYED, displayed); }

inline bool BaseData::isPersistent() const
{ return getFlag(FLAG_PERSISTENT); }

inline void BaseData::setPersistent(bool persistent)
{ setFlag(FLAG_PERSISTENT, persistent); }

inline bool BaseData::isInput() const
{ return getFlag(FLAG_INPUT); }

inline void BaseData::setInput(bool input)
{ setFlag(FLAG_INPUT, input); }

inline bool BaseData::isOutput() const
{ return getFlag(FLAG_OUTPUT); }

inline void BaseData::setOutput(bool output)
{ setFlag(FLAG_OUTPUT, output); }

inline PandaObject* BaseData::getOwner() const
{ return m_owner; }

inline void BaseData::setOwner(PandaObject* owner)
{ m_owner = owner; }

inline BaseData* BaseData::getParent() const
{ return m_parentBaseData; }

inline const types::AbstractDataTrait* BaseData::getDataTrait() const
{ return m_dataTrait; }

inline VoidDataAccessor BaseData::getVoidAccessor()
{ return VoidDataAccessor(this); }

inline void BaseData::setFlag(DataFlagsEnum flag, bool b)
{ if(b) m_dataFlags |= (DataFlags)flag; else m_dataFlags &= ~(DataFlags)flag; }

inline bool BaseData::getFlag(DataFlagsEnum flag) const
{ return m_dataFlags & (DataFlags)flag; }

} // namespace panda

#endif // BASEDATA_H
