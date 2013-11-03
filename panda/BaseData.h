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
	BaseData* data;
	void* value;
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

	const QString getName() const { return name; }
	void setName(const QString& n);
	const QString getHelp() const { return help; }
	void setHelp(const QString& h) { help = h; }
	const QString getGroup() const { return group; }
	void setGroup(const QString& g) { group = g; }
	const QString getWidget() const { return widget; }
	void setWidget(const QString& w) { widget = w; }

	bool isSet() const { return isValueSet; }
	void unset() { isValueSet = false; }
	void forceSet() { isValueSet = true; }

	int getCounter() { return counter; }

	bool isReadOnly() const { return readOnly; }
	void setReadOnly(bool b) { readOnly = b; }
	bool isDisplayed() const { return displayed; }
	void setDisplayed(bool b) { displayed = b; }
	bool isPersistent() const { return persistent; }
	void setPersistent(bool b) { persistent = b; }
	bool isInput() const { return input; }
	void setInput(bool b) { input = b; }
	bool isOutput() const { return output; }
	void setOutput(bool b) { output = b; }

	PandaObject* getOwner() const { return owner; }
	void setOwner(PandaObject* o) { owner = o; }

	virtual bool validParent(const BaseData *parent) const;
	virtual void setParent(BaseData* parent);
	BaseData* getParent() const { return parentBaseData; }

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

	bool readOnly, displayed, persistent, input, output;
	bool isValueSet;
	bool setParentProtection;
	int counter;
	QString name, help, group, widget;
	PandaObject* owner;
	BaseData* parentBaseData;

private:
	BaseData() {}
};

} // namespace panda

#endif // BASEDATA_H
