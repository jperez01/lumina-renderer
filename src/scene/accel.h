//
// Created by juperez on 5/26/23.
//

#pragma once

#include "primitives/mesh.h"

LUMINA_NAMESPACE_BEGIN

struct Node {
    BoundingBox3f box;
    std::vector<Node*> children;
    std::vector<uint32_t> triangle_indices;
    std::vector<uint32_t> mesh_indices;

    Node() = default;
};

static constexpr int MAX_RECURSIVE_DEPTH = 12;
static constexpr int MAX_TRIANGLES_PER_NODE = 10;

class Accel {
public:
    void addMesh(Mesh* mesh);

    void build();
    const BoundingBox3f &getBoundingBox() const { return m_bbox; }

    bool rayIntersect(const Ray3f& ray, Intersection& its, bool shadowRay) const;
private:
    std::vector<Mesh *> m_meshes;
    BoundingBox3f m_bbox;
    Node* m_root;

    bool intersectRecursive(const Node &node, Ray3f& ray, Intersection& its, bool shadowRay, uint32_t& hit_index) const;
    Node* build(BoundingBox3f& box, std::vector<uint32_t>& triangle_indices,
                std::vector<uint32_t>& mesh_indices, int recursiveDepth = 0);
};

std::vector<BoundingBox3f> subdivideBox(BoundingBox3f& parent);
LUMINA_NAMESPACE_END
