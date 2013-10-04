#ifndef BASEDATA_H
#define BASEDATA_H

#include <panda/DataNode.h>
#include <QString>
#include <QDomDocument>

#include <typeinfo>

namespace panda
{

class PandaObject;
class DataTypeId;

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

	void update();

	virtual bool isSingleValue() const = 0;
	virtual bool isVector() const = 0;
	virtual bool isAnimation() const = 0;

	virtual int getValueType() const = 0;
	virtual int getFullType() const;

	template <class T> static int getValueTypeOf() { return DataTypeId::getIdOf<T>(); }
	static int getFullTypeOfSingleValue(int valueType);
	static int getFullTypeOfVector(int valueType);
	static int getFullTypeOfAnimation(int valueType);

	// To decode the number given by getFullType
	static int getValueType(int fullType);
	static bool isSingleValue(int fullType);
	static bool isVector(int fullType);
	static bool isAnimation(int fullType);
	static int replaceValueType(int fullType, int newType);

	virtual int getSize() const = 0;
	virtual void clear(int size = 0, bool init = false) = 0;

	virtual bool isNumerical() const = 0;	// Is the data a simple numerical value, that we can convert to other number types?
	virtual double getNumerical(int index) const = 0;
	virtual void setNumerical(double val, int index) = 0;
	virtual void* getValueVoidPtr() = 0;

	QString toString() const;
	virtual void fromString(const QString& text) = 0;

	virtual QString getValueTypeName() const = 0;
	virtual QString getValueTypeNamePlural() const = 0;
	virtual QString getDescription() const;

	virtual void copyValueFrom(const BaseData* parent) = 0;

	virtual void save(QDomDocument& doc, QDomElement& elem) = 0;
	virtual void load(QDomElement& elem) = 0;

protected:
	virtual void doAddInput(DataNode* node);
	virtual void doRemoveInput(DataNode* node);
	virtual void doAddOutput(DataNode* node);
	virtual void doRemoveOutput(DataNode* node);

	virtual QString doToString() const = 0;

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

//***************************************************************//

class DataTypeId
{
public:
	static int getId(const std::type_info& type);
	template <class T>
	static int getIdOf() { return getId(typeid(T)); }

private:
	DataTypeId();
};

} // namespace panda

#endif // BASEDATA_H
