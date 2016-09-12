#include <panda/XmlDocument.h>

#include <tinyxml2.h>

namespace panda
{

XmlAttribute::XmlAttribute(const tinyxml2::XMLAttribute* attribute)
	: m_attribute(attribute)
{
}

XmlAttribute::operator bool() const
{
	return m_attribute != nullptr;
}

std::string XmlAttribute::name() const
{
	if (m_attribute)
		return m_attribute->Name();
	return "";
}

bool XmlAttribute::toBool() const
{
	if (m_attribute)
		return m_attribute->BoolValue();
	return false;
}

int XmlAttribute::toInt() const
{
	if (m_attribute)
		return m_attribute->IntValue();
	return 0;
}

unsigned int XmlAttribute::toUnsigned() const
{
	if (m_attribute)
		return m_attribute->UnsignedValue();
	return 0;
}

float XmlAttribute::toFloat() const
{
	if (m_attribute)
		return m_attribute->FloatValue();
	return 0;
}

double XmlAttribute::toDouble() const
{
	if (m_attribute)
		return m_attribute->DoubleValue();
	return 0;
}

std::string XmlAttribute::toString() const
{
	if (m_attribute)
		return m_attribute->Value();
	return "";
}

//****************************************************************************//

XmlElement::XmlElement(tinyxml2::XMLElement* element)
	: m_element(element)
{
}

XmlElement::operator bool() const
{
	return m_element != nullptr;
}

std::string XmlElement::name() const
{
	if (m_element)
		return m_element->Name();
	return "";
}

void XmlElement::setName(const std::string& name)
{
	if (m_element)
		m_element->SetName(name.c_str());
}

XmlElement XmlElement::firstChild(const std::string& name) const
{
	if (m_element)
		return XmlElement(m_element->FirstChildElement(name.empty() ? nullptr : name.c_str()));
	return XmlElement();
}

XmlElement XmlElement::nextSibling(const std::string& name) const
{
	if (m_element)
		return XmlElement(m_element->NextSiblingElement(name.empty() ? nullptr : name.c_str()));
	return XmlElement();
}

XmlElement XmlElement::addChild(const std::string& name)
{
	if (m_element)
	{
		auto newElement = m_element->GetDocument()->NewElement(name.c_str());
		m_element->InsertEndChild(newElement);
		return XmlElement(newElement);
	}
	return XmlElement();
}

void XmlElement::removeChild(XmlElement& child)
{
	if (m_element && child.m_element)
	{
		m_element->DeleteChild(child.m_element);
		child.m_element = nullptr;
	}
}

XmlAttribute XmlElement::attribute(const std::string& name) const
{
	if (m_element)
		return XmlAttribute(const_cast<const tinyxml2::XMLElement*>(m_element)->FindAttribute(name.c_str()));
	return XmlAttribute();
}

void XmlElement::removeAttribute(const std::string& name)
{
	if (m_element)
		m_element->DeleteAttribute(name.c_str());
}

void XmlElement::setAttribute(const std::string& name, bool value)
{
	if (m_element)
		m_element->SetAttribute(name.c_str(), value);
}

void XmlElement::setAttribute(const std::string& name, int value)
{
	if (m_element)
		m_element->SetAttribute(name.c_str(), value);
}

void XmlElement::setAttribute(const std::string& name, unsigned int value)
{
	if (m_element)
		m_element->SetAttribute(name.c_str(), value);
}

void XmlElement::setAttribute(const std::string& name, float value)
{
	if (m_element)
		m_element->SetAttribute(name.c_str(), value);
}

void XmlElement::setAttribute(const std::string& name, double value)
{
	if (m_element)
		m_element->SetAttribute(name.c_str(), value);
}

void XmlElement::setAttribute(const std::string& name, const std::string& value)
{
	if (m_element)
		m_element->SetAttribute(name.c_str(), value.c_str());
}

std::string XmlElement::text() const
{
	if (m_element)
	{
		auto text = m_element->GetText();
		return text ? text : "";
	}
	return "";
}

void XmlElement::setText(const std::string& text)
{
	if (m_element)
		m_element->SetText(text.c_str());
}

//****************************************************************************//

XmlDocument::XmlDocument()
	: m_document(std::make_shared<tinyxml2::XMLDocument>())
{
}

bool XmlDocument::loadFromFile(const std::string& path)
{
	return tinyxml2::XML_SUCCESS == m_document->LoadFile(path.c_str());
}

bool XmlDocument::loadFromMemory(const std::string& xmlContent)
{
	return tinyxml2::XML_SUCCESS == m_document->Parse(xmlContent.c_str(), xmlContent.size());
}

bool XmlDocument::saveToFile(const std::string& path)
{
	return tinyxml2::XML_SUCCESS == m_document->SaveFile(path.c_str());
}

std::string XmlDocument::saveToMemory()
{
	tinyxml2::XMLPrinter printer;
	m_document->Print(&printer);
	return printer.CStr();
}

XmlElement XmlDocument::root() const
{
	auto element = m_document->RootElement();
	if (!element) // TODO: verify if document can have no root
	{
		element = m_document->NewElement("xml");
		m_document->InsertEndChild(element);
	}
	return XmlElement(element);
}

void XmlDocument::clear()
{
	m_document->Clear();
}

} // namespace panda
