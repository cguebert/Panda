#pragma once

#include <panda/core.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace panda {

class ObjectsList;
class PandaDocument;
class PandaObject;
class XmlElement;

namespace gui
{
	class BaseGUI;
}

namespace serialization
{

using Objects = std::vector<panda::PandaObject*>;
enum class DocumentType { Base, Rendered, Interactive };

std::string PANDA_CORE_API getDocumentName(DocumentType type);

DocumentType PANDA_CORE_API getDocumentType(const std::string& name);
DocumentType PANDA_CORE_API getDocumentType(panda::PandaDocument* document);

std::unique_ptr<panda::PandaDocument> PANDA_CORE_API createDocument(DocumentType type, panda::gui::BaseGUI& gui);
bool PANDA_CORE_API canImport(DocumentType current, DocumentType import);

bool PANDA_CORE_API writeFile(PandaDocument* document, const std::string& fileName);
std::unique_ptr<PandaDocument> PANDA_CORE_API readFile(const std::string& fileName, panda::gui::BaseGUI& gui);
Objects PANDA_CORE_API importFile(PandaDocument* document, ObjectsList& objectsList, const std::string& fileName);

std::string PANDA_CORE_API writeTextDocument(PandaDocument* document, const Objects& objects);
Objects PANDA_CORE_API readTextDocument(PandaDocument* document, ObjectsList& objectsList, const std::string& text);

bool PANDA_CORE_API saveDoc(PandaDocument* document, XmlElement& root, const Objects& objects);
Objects PANDA_CORE_API loadDoc(PandaDocument* document, ObjectsList& objectsList, const XmlElement& root);

} // namespace serialization

} // namespace panda

