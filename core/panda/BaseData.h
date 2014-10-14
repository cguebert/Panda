#ifndef BASEDATA_H
#define BASEDATA_H

#include <panda/DataNode.h>
#include <typeinfo>

#include <QString>
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
		BaseInitData() : data(nullptr), owner(nullptr) {}
		BaseData* data;
		PandaObject* owner;
		QString name, help;
	};

	explicit BaseData(const BaseInitData& init);
	BaseData(const QString& name, const QString& help, PandaObject* owner);
	virtual ~BaseData() {}

	const QString getName() const;	/// Name used in the UI and for saving / loading
	void setName(const QString& name);
	const QString getHelp() const;	/// Message describing the Data
	void setHelp(const QString& help);
	const QString getWidget() const; /// Custom widget to use for this Data
	void setWidget(const QString& widget);
	const QString getWidgetData() const; /// Some custom widgets need parameters, they are saved in string format
	void setWidgetData(const QString& widgetData);

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
	bool isInput() const;		/// Is it used as of one the inputs of a PandaObject
	void setInput(bool input);
	bool isOutput() const;		/// Is it used as of one the outputs of a PandaObject
	void setOutput(bool output);

	PandaObject* getOwner() const; /// The PandaObject that owns this Data
	void setOwner(PandaObject* owner);

	virtual bool validParent(const BaseData* parent) const; /// Can parent be connected to this Data
	virtual void setParent(BaseData* parent); /// Set the other Data as the parent to this one (its value will be copied each time it changes)
	BaseData* getParent() const; /// Returns the current parent, or nullptr

	virtual const types::AbstractDataTrait* getDataTrait() const; /// Return a class describing the type stored in this Data
	virtual const void* getVoidValue() const = 0; /// Return a void* pointing to the value (use the DataTrait to exploit it)
	VoidDataAccessor getVoidAccessor(); /// Return a wrapper around the void*, that will call endEdit when destroyed

	virtual QString getDescription() const; /// Get a readable name of the type stored in this Data

	virtual void copyValueFrom(const BaseData* parent); /// Copy the value from parent to this Data

	virtual void save(QDomDocument& doc, QDomElement& elem); /// Save the value of the Data in a Xml node
	virtual void load(QDomElement& elem); /// Load the value from Xml

	virtual void setDirtyValue(const DataNode* caller); // We override this function in order to log the event

protected:
	virtual void doAddInput(DataNode* node);
	virtual void doRemoveInput(DataNode* node);
	virtual void doAddOutput(DataNode* node);
	virtual void doRemoveOutput(DataNode* node);

	friend class VoidDataAccessor;
	virtual void* beginVoidEdit() = 0;
	virtual void endVoidEdit() = 0;

	void initInternals(const std::type_info& type);

	bool m_readOnly, m_displayed, m_persistent, m_input, m_output;
	bool m_isValueSet;
	bool m_setParentProtection;
	int m_counter;
	QString m_name, m_help, m_widget, m_widgetData;
	PandaObject* m_owner;
	BaseData* m_parentBaseData;
	types::AbstractDataTrait* m_dataTrait;
	AbstractDataCopier* m_dataCopier;

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

inline const QString BaseData::getName() const
{ return m_name; }

inline void BaseData::setName(const QString& name)
{ m_name = name; }

inline const QString BaseData::getHelp() const
{ return m_help; }

inline void BaseData::setHelp(const QString& help)
{ m_help = help; }

inline const QString BaseData::getWidget() const
{ return m_widget; }

inline void BaseData::setWidget(const QString& widget)
{ m_widget = widget; }

inline const QString BaseData::getWidgetData() const
{ return m_widgetData; }

inline void BaseData::setWidgetData(const QString& widgetData)
{ m_widgetData = widgetData; }

inline bool BaseData::isSet() const
{ return m_isValueSet; }

inline void BaseData::unset()
{ m_isValueSet = false; }

inline void BaseData::forceSet()
{ m_isValueSet = true; }

inline int BaseData::getCounter() const
{ if(m_parentBaseData) return m_parentBaseData->getCounter(); return m_counter; }

inline bool BaseData::isReadOnly() const
{ return m_readOnly; }

inline void BaseData::setReadOnly(bool readOnly)
{ m_readOnly = readOnly; }

inline bool BaseData::isDisplayed() const
{ return m_displayed; }

inline void BaseData::setDisplayed(bool displayed)
{ m_displayed = displayed; }

inline bool BaseData::isPersistent() const
{ return m_persistent; }

inline void BaseData::setPersistent(bool persistent)
{ m_persistent = persistent; }

inline bool BaseData::isInput() const
{ return m_input; }

inline void BaseData::setInput(bool input)
{ m_input = input; }

inline bool BaseData::isOutput() const
{ return m_output; }

inline void BaseData::setOutput(bool output)
{ m_output = output; }

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

} // namespace panda

#endif // BASEDATA_H
