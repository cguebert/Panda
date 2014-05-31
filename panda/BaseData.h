#ifndef BASEDATA_H
#define BASEDATA_H

#include <panda/DataNode.h>
#include <QString>
#include <QDomDocument>

namespace panda
{

class PandaObject;
class BaseData;
namespace types { class AbstractDataTrait; }

class VoidDataAccessor
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

//***************************************************************//

class BaseData : public DataNode
{
public:
	PANDA_CLASS(BaseData, DataNode)
	class BaseInitData
	{
	public:
		BaseInitData()
			: data(nullptr)
			, owner(nullptr)
		{}
		BaseData* data;
		PandaObject* owner;
		QString name, help;
	};

	explicit BaseData(const BaseInitData& init);
	BaseData(const QString& name, const QString& help, PandaObject* owner);
	virtual ~BaseData() {}

	const QString getName() const { return m_name; }
	void setName(const QString& name) { m_name = name; }
	const QString getHelp() const { return m_help; }
	void setHelp(const QString& help) { m_help = help; }
	const QString getGroup() const { return m_group; }
	void setGroup(const QString& group) { m_group = group; }
	const QString getWidget() const { return m_widget; }
	void setWidget(const QString& widget) { m_widget = widget; }
	const QString getWidgetData() const { return m_widgetData; }
	void setWidgetData(const QString& widgetData) { m_widgetData = widgetData; }

	bool isSet() const { return m_isValueSet; }
	void unset() { m_isValueSet = false; }
	void forceSet() { m_isValueSet = true; }

	virtual int getCounter() const { return m_counter; }

	bool isReadOnly() const { return m_readOnly; }
	void setReadOnly(bool readOnly) { m_readOnly = readOnly; }
	bool isDisplayed() const { return m_displayed; }
	void setDisplayed(bool displayed) { m_displayed = displayed; }
	bool isPersistent() const { return m_persistent; }
	void setPersistent(bool persistent) { m_persistent = persistent; }
	bool isInput() const { return m_input; }
	void setInput(bool input) { m_input = input; }
	bool isOutput() const { return m_output; }
	void setOutput(bool output) { m_output = output; }

	PandaObject* getOwner() const { return m_owner; }
	void setOwner(PandaObject* owner) { m_owner = owner; }

	virtual bool validParent(const BaseData *parent) const;
	virtual void setParent(BaseData* parent);
	BaseData* getParent() const { return m_parentBaseData; }

	virtual const types::AbstractDataTrait* getDataTrait() const = 0;
	virtual const void* getVoidValue() const = 0;
	VoidDataAccessor getVoidAccessor() { return VoidDataAccessor(this); }

	virtual QString getDescription() const;

	virtual void copyValueFrom(const BaseData* parent) = 0;

	virtual void save(QDomDocument& doc, QDomElement& elem);
	virtual void load(QDomElement& elem);

protected:
	virtual void doAddInput(DataNode* node);
	virtual void doRemoveInput(DataNode* node);
	virtual void doAddOutput(DataNode* node);
	virtual void doRemoveOutput(DataNode* node);

	friend class VoidDataAccessor;
	virtual void* beginVoidEdit() = 0;
	virtual void endVoidEdit() = 0;

	void initFlags();

	bool m_readOnly, m_displayed, m_persistent, m_input, m_output;
	bool m_isValueSet;
	bool m_setParentProtection;
	int m_counter;
	QString m_name, m_help, m_group, m_widget, m_widgetData;
	PandaObject* m_owner;
	BaseData* m_parentBaseData;

private:
	BaseData() {}
};

} // namespace panda

#endif // BASEDATA_H
