//
// Created by juperez on 5/25/23.
//

#include <fstream>
#include "objMesh.h"
#include "utils/timer.h"

LUMINA_NAMESPACE_BEGIN

WavefrontObj::WavefrontObj(const lumina::PropertyList &propsList) {
    typedef std::unordered_map<OBJVertex, uint32_t, OBJVertexHash> VertexMap;

    std::filesystem::path filename =
            getFileResolver()->resolve(propsList.getString("filename"));

    std::ifstream is(filename.string());
    if (is.fail())
        throw LuminaException("Unable to open OBJ file %s!", filename);
    Transform transform = propsList.getTransform("toWorld", Transform());

    std::cout << "Loading " << filename << "...";
    std::cout.flush();
    Timer timer;

    std::vector<Vector3f> positions, normals;
    std::vector<Vector2f> texCoords;
    std::vector<uint32_t> indices;
    std::vector<OBJVertex> vertices;
    VertexMap vertexMap;

    std::string line_str;
    while (std::getline(is, line_str)) {
        std::istringstream line(line_str);

        std::string prefix;
        line >> prefix;

        if (prefix == "v") {
            Point3f p;
            line >> p.x() >> p.y() >> p.z();
            p = transform * p;

            m_bbox.expandBy(p);
            positions.push_back(p);
        } else if (prefix == "vt") {
            Point2f tc;
            line >> tc.x() >> tc.y();
            texCoords.push_back(tc);
        } else if (prefix == "vn") {
            Normal3f n;
            line >> n.x() >> n.y() >> n.z();

            normals.push_back((transform * n).normalized());
        } else if (prefix == "f") {
            std::string v1, v2, v3, v4;
            line >> v1 >> v2 >> v3 >> v4;
            OBJVertex verts[6];
            int nVertices = 3;

            verts[0] = OBJVertex(v1);
            verts[1] = OBJVertex(v2);
            verts[2] = OBJVertex(v3);

            if (!v4.empty()) {
                verts[3] = OBJVertex(v4);
                verts[4] = verts[0];
                verts[5] = verts[2];
                nVertices = 6;
            }

            for (int i =0; i < nVertices; i++) {
                const OBJVertex& v = verts[i];
                auto it = vertexMap.find(v);
                if (it == vertexMap.end()) {
                    vertexMap[v] = (uint32_t) vertices.size();
                    indices.push_back((uint32_t) vertices.size());
                    vertices.push_back(v);
                } else {
                    indices.push_back(it->second);
                }
            }
        }
    }

    m_faces.resize(3, indices.size()/3);
    memcpy(m_faces.data(), indices.data(), sizeof(uint32_t) * indices.size());

    m_vertices.resize(3, vertices.size());
    for (uint32_t i = 0; i < vertices.size(); i++) {
        m_vertices.col(i) = positions.at(vertices[i].p - 1);
    }

    if (!normals.empty()) {
        m_normals.resize(3, normals.size());
        for (uint32_t i = 0; i < normals.size(); i++) {
            m_vertices.col(i) = normals.at(vertices[i].n - 1);
        }
    }

    if (!texCoords.empty()) {
        m_uvs.resize(3, texCoords.size());
        for (uint32_t i = 0; i < texCoords.size(); i++) {
            m_uvs.col(i) = texCoords.at(vertices[i].uv - 1);
        }
    }

    m_name = filename.string();
    std::cout << "done. (V=" << m_vertices.cols() << ", F=" << m_faces.cols() << ", took "
              << timer.elapsedString() << " and "
              << memString(m_faces.size() * sizeof(uint32_t) +
                           sizeof(float) * (m_vertices.size() + m_normals.size() + m_uvs.size()))
              << ")" << std::endl;
}

LUMINA_REGISTER_CLASS(WavefrontObj, "obj")
LUMINA_NAMESPACE_END
