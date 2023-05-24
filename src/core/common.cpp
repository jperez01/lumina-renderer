//
// Created by juperez on 5/23/23.
//

#include "common.h"

LUMINA_NAMESPACE_BEGIN

std::vector<std::string> tokenize(const std::string &s, const std::string &delim, bool includeEmpty) {
    std::vector<std::string> tokens;

    std::string::size_type lastPos = 0, position = s.find_first_of(delim, lastPos);
    while (lastPos != std::string::npos) {
        if (position != lastPos || includeEmpty) {
            tokens.push_back(s.substr(lastPos, position + lastPos));
        }
        lastPos = position;
        if (lastPos != std::string::npos) {
            lastPos += 1;
            position = s.find_first_of(delim, lastPos);
        }
    }

    return tokens;
}

Eigen::Vector3f toVector3f(const std::string &str) {
    std::vector<std::string> tokens = tokenize(str);
    if (tokens.size() != 3)
        std::cout << "Expected 3 values \n";

    Eigen::Vector3f result;
    for (int i = 0; i < 3; i++)
        result[i] = toFloat(tokens[i]);

    return result;
}

    float toFloat(const std::string &str) {
        char* end_ptr = nullptr;
        float value = strtof(str.c_str(), &end_ptr);

        if (*end_ptr != '\0')
            std::cout << "Could not parse floating point from string \n";

        return value;
    }

    int toInt(const std::string &str) {
        char* end_ptr = nullptr;
        int value = (int) strtol(str.c_str(), &end_ptr, 10);

        if (*end_ptr != '\0')
            std::cout << "Could not parse floating point from string \n";

        return value;
    }

    unsigned int toUInt(const std::string &str) {
        char* end_ptr = nullptr;
        unsigned int value = (unsigned int) strtol(str.c_str(), &end_ptr, 10);

        if (*end_ptr != '\0')
            std::cout << "Could not parse floating point from string \n";

        return value;
    }

    bool toBool(const std::string &str) {
        if (str == "false")
            return false;
        else if (str == "true")
            return true;
        else
            std::cout << "Could not parse bool from string \n";

        return false;
    }

LUMINA_NAMESPACE_END