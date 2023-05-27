//
// Created by juperez on 5/24/23.
//

#pragma once

#include "core/common.h"
#include "core/object.h"

#include <pugixml.hpp>
#include <set>

LUMINA_NAMESPACE_BEGIN

std::string offset(std::string& filename, ptrdiff_t pos);
void check_attributes(std::string& filename, const pugi::xml_node& node, std::set<std::string> attrs);
LuminaObject* loadXMLFile(std::string& filename);

LUMINA_NAMESPACE_END
