#ifndef XMLDOCUMENT_H
#define XMLDOCUMENT_H

#include <panda/core.h>

#include <memory>
#include <string>

namespace tinyxml2
{
class XMLAttribute;
class XMLElement;
class XMLDocument;
}

namespace panda
{

class PANDA_CORE_API XmlAttribute
{
public:
	XmlAttribute(const tinyxml2::XMLAttribute* attribute = nullptr);

	explicit operator bool() const; // Test if valid

	std::string name() const;

	bool toBool() const;
	int toInt() const;
	unsigned int toUnsigned() const;
	float toFloat() const;
	double toDouble() const;
	std::string toString() const;

private:
	const tinyxml2::XMLAttribute* m_attribute = nullptr;
};

class PANDA_CORE_API XmlElement
{
public:
	XmlElement(tinyxml2::XMLElement* element = nullptr);
	explicit operator bool() const; // Test if valid

	std::string name() const;
	void setName(const std::string& name);

	XmlElement firstChild(const std::string& name = "");
	XmlElement nextSibling(const std::string& name = "");
	XmlElement addChild(const std::string& name);
	void removeChild(XmlElement& child);

	XmlAttribute attribute(const std::string& name) const;
	void removeAttribute(const std::string& name);

	void setAttribute(const std::string& name, bool value);
	void setAttribute(const std::string& name, int value);
	void setAttribute(const std::string& name, unsigned int value);
	void setAttribute(const std::string& name, float value);
	void setAttribute(const std::string& name, double value);
	void setAttribute(const std::string& name, const std::string& value);

	std::string text() const;
	void setText(const std::string& text);

private:
	tinyxml2::XMLElement* m_element = nullptr;
};

class PANDA_CORE_API XmlDocument
{
public:
	XmlDocument();

	bool loadFromFile(const std::string& path);
	bool loadFromMemory(const std::string& xmlContent);

	bool saveToFile(const std::string& path);
	std::string saveToMemory();

	XmlElement root() const;
	void clear();

private:
	std::shared_ptr<tinyxml2::XMLDocument> m_document;
};

} // namespace panda

#endif // XMLDOCUMENT_H
