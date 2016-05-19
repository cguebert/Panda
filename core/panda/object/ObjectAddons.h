#ifndef OBJECTADDONS_H
#define OBJECTADDONS_H

#include <panda/core.h>

#include <memory>
#include <vector>

namespace panda
{

class PandaObject;
class XmlElement;

class PANDA_CORE_API BaseObjectAddon
{
public:
	using SPtr = std::shared_ptr<BaseObjectAddon>;

	BaseObjectAddon(PandaObject& object);
	virtual ~BaseObjectAddon();

	virtual void save(XmlElement& elem) = 0;
	virtual bool load(XmlElement& elem) = 0;

protected:
	PandaObject& m_object;
};

class PANDA_CORE_API ObjectAddons
{
public:
	explicit ObjectAddons(PandaObject& object);

	void save(XmlElement& elem);
	void load(XmlElement& elem);

private:
	void createAddons();

	PandaObject& m_object;	
	std::vector<BaseObjectAddon::SPtr> m_addons;
};

//****************************************************************************//

class PANDA_CORE_API BaseObjectAddonCreator
{
public:
	using SPtr = std::shared_ptr<BaseObjectAddonCreator>;

	virtual ~BaseObjectAddonCreator() {}
	virtual BaseObjectAddon::SPtr create(PandaObject& object) = 0;
};

template <class T>
class ObjectAddonCreator
{
public:
	virtual BaseObjectAddon::SPtr create(PandaObject& object)
	{ return std::make_shared<T>(object); }
};

class PANDA_CORE_API ObjectAddonsRegistry
{
public:
	static ObjectAddonsRegistry& instance();

	template <class T>
	int addObjectAddon()
	{ 
		m_creators.push_back(std::make_shared<ObjectAddonCreator<T>>());
		return 0;
	}

	using Creators = std::vector<BaseObjectAddonCreator::SPtr>;
	const Creators& getCreators() const;
	
private:
	ObjectAddonsRegistry();

	Creators m_creators;
};

} // namespace Panda

#endif // OBJECTADDONS_H
