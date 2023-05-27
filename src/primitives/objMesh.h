//
// Created by juperez on 5/25/23.
//

#pragma once

#include "mesh.h"

LUMINA_NAMESPACE_BEGIN

class WavefrontObj : public Mesh {
public:
    WavefrontObj(const PropertyList& propsList);

protected:
    struct OBJVertex {
        uint32_t p = (uint32_t) -1;
        uint32_t n = (uint32_t) -1;
        uint32_t uv = (uint32_t) -1;

        inline OBJVertex() {}

        inline  OBJVertex(const std::string& string) {
            std::vector<std::string> tokens = tokenize(string, "/", true);

            if (tokens.size() < 1 || tokens.size() > 3)
                throw LuminaException("Invalid vertex data: %s", string);

            p = toUInt(tokens[0]);
            if (tokens.size() >= 2 && !tokens[1].empty())
                uv = toUInt(tokens[1]);

            if (tokens.size() >= 3 && !tokens[2].empty())
                n = toUInt(tokens[2]);
        }

        inline bool operator==(const OBJVertex &v) const {
            return v.p == p && v.n == n && v.uv == uv;
        }
    };

    struct OBJVertexHash {
        std::size_t operator()(const OBJVertex& v) const {
            size_t hash = std::hash<uint32_t>()(v.p);
            hash = hash * 37 + std::hash<uint32_t>()(v.n);
            hash = hash * 37 + std::hash<uint32_t>()(v.uv);

            return hash;
        }
    };
};

LUMINA_NAMESPACE_END
