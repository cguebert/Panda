#pragma once

#include <panda/core.h>

#include <string>
#include <utility>
#include <vector>

namespace panda {

class PandaDocument;
class PandaObject;
class XmlElement;

namespace serialization
{

using ObjectsList = std::vector<panda::PandaObject*>;
using LoadResult = std::pair<bool, ObjectsList>;

bool PANDA_CORE_API writeFile(PandaDocument* document, const std::string& fileName);
LoadResult PANDA_CORE_API readFile(PandaDocument* document, const std::string& fileName, bool isImport = false);

std::string PANDA_CORE_API writeTextDocument(PandaDocument* document, const ObjectsList& objects);
LoadResult PANDA_CORE_API readTextDocument(PandaDocument* document, const std::string& text);

bool PANDA_CORE_API saveDoc(PandaDocument* document, XmlElement& root, const ObjectsList& objects);
LoadResult PANDA_CORE_API loadDoc(PandaDocument* document, XmlElement& root);

} // namespace serialization

} // namespace panda
