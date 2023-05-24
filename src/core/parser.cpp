//
// Created by juperez on 5/24/23.
//

#include "parser.h"
#include <pugixml.hpp>

void lumina::loadXMLFile(const std::string &filename) {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(filename.c_str());
    if (!result) {

    }
}
