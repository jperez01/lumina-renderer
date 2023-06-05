//
// Created by juperez on 5/26/23.
//

#include "accel.h"

#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <tbb/task_arena.h>
#include <Eigen/Geometry>
#include <stack>
#include <queue>

LUMINA_NAMESPACE_BEGIN

    void Accel::addMesh(Mesh *mesh) {
        m_meshes.push_back(mesh);
        m_bbox.expandBy(mesh->getBoundingBox());
    }

    void Accel::build() {
        /* Nothing to do here for now */
        uint32_t total_triangles = 0;
        for (uint32_t i = 0; i < m_meshes.size(); i++) {
            total_triangles += m_meshes.at(i)->getTriangleCount();
        }

        uint32_t offset = 0;
        std::vector<uint32_t> triangles(total_triangles);
        std::vector<uint32_t> mesh_indices(total_triangles);
        for (uint32_t current_mesh = 0; current_mesh < m_meshes.size(); current_mesh++) {
            uint32_t num_triangles_in_mesh = m_meshes.at(current_mesh)->getTriangleCount();
            for (uint32_t i = 0; i < num_triangles_in_mesh; i++) {
                triangles[i + offset] = i;
                mesh_indices[i + offset] = current_mesh;
            }
            offset += num_triangles_in_mesh;
        }

        m_root = build(m_bbox, triangles, mesh_indices);

        flattenTree();
    }

    bool Accel::rayIntersect(const Ray3f &ray_, Intersection &its, bool shadowRay) const {
        bool foundIntersection = false;  // Was an intersection found so far?
        uint32_t f = (uint32_t) -1;      // Triangle index of the closest intersection

        Ray3f ray(ray_); /// Make a copy of the ray (we will need to update its '.maxt' value)
        /* Brute force search through all triangles */
         foundIntersection = intersectRecursive(*m_root, ray, its, shadowRay, f);
        // foundIntersection = intersectIterative(ray, its, shadowRay, f);
        if (shadowRay)
            return foundIntersection;

        if (foundIntersection) {
            /* At this point, we now know that there is an intersection,
               and we know the triangle index of the closest such intersection.

               The following computes a number of additional properties which
               characterize the intersection (normals, texture coordinates, etc..)
            */

            /* Find the barycentric coordinates */
            Vector3f bary;
            bary << 1-its.uv.sum(), its.uv;

            /* References to all relevant mesh buffers */
            const Mesh *mesh   = its.mesh;
            const MatrixXf &V  = mesh->getVertexPositions();
            const MatrixXf &N  = mesh->getVertexNormals();
            const MatrixXf &UV = mesh->getVertexTexCoords();
            const MatrixXu &F  = mesh->getIndices();

            /* Vertex indices of the triangle */
            uint32_t idx0 = F(0, f), idx1 = F(1, f), idx2 = F(2, f);

            Point3f p0 = V.col(idx0), p1 = V.col(idx1), p2 = V.col(idx2);

            /* Compute the intersection positon accurately
               using barycentric coordinates */
            its.p = bary.x() * p0 + bary.y() * p1 + bary.z() * p2;

            /* Compute proper texture coordinates if provided by the mesh */
            if (UV.size() > 0)
                its.uv = bary.x() * UV.col(idx0) +
                         bary.y() * UV.col(idx1) +
                         bary.z() * UV.col(idx2);

            /* Compute the geometry frame */
            its.geoFrame = Frame((p1-p0).cross(p2-p0).normalized());

            if (N.size() > 0) {
                /* Compute the shading frame. Note that for simplicity,
                   the current implementation doesn't attempt to provide
                   tangents that are continuous across the surface. That
                   means that this code will need to be modified to be able
                   use anisotropic BRDFs, which need tangent continuity */

                its.shadingFrame = Frame(
                        (bary.x() * N.col(idx0) +
                         bary.y() * N.col(idx1) +
                         bary.z() * N.col(idx2)).normalized());
            } else {
                its.shadingFrame = its.geoFrame;
            }
        }

        return foundIntersection;
    }

    bool Accel::intersectIterative(Ray3f& ray, Intersection& its, bool shadowRay, uint32_t& hit_index) const
    {
        bool foundIntersection = false;
        std::stack<Node*> queue;
        queue.push(m_root);

        while (!queue.empty()) {
            Node currentNode = *queue.top();
            queue.pop();

            if (currentNode.box.rayIntersect(ray)) {
                for (uint32_t i = 0; i < currentNode.triangle_indices.size(); i++) {
                    uint32_t triangle_index = currentNode.triangle_indices[i];
                    uint32_t mesh_index = currentNode.mesh_indices[i];

                    float u, v, t;
                    if (m_meshes[mesh_index]->rayIntersect(triangle_index, ray, u, v, t) && t < ray.maxt) {
                        if (shadowRay)
                            return true;

                        ray.maxt = its.t = t;
                        its.uv = Point2f(u, v);
                        its.mesh = m_meshes[mesh_index];
                        hit_index = triangle_index;
                        foundIntersection = true;
                    }
                }
                if (foundIntersection && shadowRay)
                    return true;

                if (!currentNode.children.empty()) {
                    std::vector<std::pair<Node*, float>> sortedNodes(currentNode.children.size());
                    for (uint32_t i = 0; i < currentNode.children.size(); i++) {
                        auto& child = currentNode.children.at(i);
                        sortedNodes[i] = std::pair<Node*, float>(child, child->box.distanceTo(ray.o));
                    }
                    std::sort(sortedNodes.begin(), sortedNodes.end(), [](const std::pair<Node*, float>& a,
                        const std::pair<Node*, float>& b) {
                            return a.second > b.second;
                        });

                    for (auto& pair : sortedNodes) {
                        queue.push(pair.first);
                    }
                }
            }
        }

        return foundIntersection;
    }

    bool Accel::intersectRecursive(const Node &node, Ray3f &ray, Intersection &its, bool shadowRay,
                                   uint32_t &hit_idx) const {
        bool foundIntersection = false;

        if (!node.box.rayIntersect(ray))
            return false;

        for (uint32_t i = 0; i < node.triangle_indices.size(); i++) {
            uint32_t triangle_index = node.triangle_indices[i];
            uint32_t mesh_index = node.mesh_indices[i];

            float u, v, t;
            if (m_meshes[mesh_index]->rayIntersect(triangle_index, ray, u, v, t) && t < ray.maxt) {
                if (shadowRay)
                    return true;

                ray.maxt = its.t = t;
                its.uv = Point2f(u, v);
                its.mesh = m_meshes[mesh_index];
                hit_idx = triangle_index;
                foundIntersection = true;
            }
        }

        if (!node.children.empty()) {
            std::vector<std::pair<Node*, float>> sortedNodes(node.children.size());
            for (uint32_t i = 0; i < node.children.size(); i++) {
                auto &child = node.children.at(i);
                sortedNodes[i] = std::pair<Node*, float>(child, child->box.distanceTo(ray.o));
            }
            std::sort(sortedNodes.begin(), sortedNodes.end(), [](const std::pair<Node*,float> &a,
                                                                 const std::pair<Node*,float> &b) {
                return a.second < b.second;
            });

            for (auto &child : sortedNodes) {
                foundIntersection = intersectRecursive(*child.first, ray, its, shadowRay, hit_idx) || foundIntersection;
                if (foundIntersection && shadowRay)
                    return true;
            }
        }

        return foundIntersection;
    }

    Node *Accel::build(BoundingBox3f &box, std::vector<uint32_t> &triangle_indices, std::vector<uint32_t> &mesh_indices, int recursive_depth) {
        if (triangle_indices.empty())
            return nullptr;

        amount++;
        if (triangle_indices.size() < MAX_TRIANGLES_PER_NODE || recursive_depth > MAX_RECURSIVE_DEPTH) {
            Node* newNode = new Node();
            newNode->triangle_indices = std::vector<uint32_t>(triangle_indices);
            newNode->mesh_indices = std::vector<uint32_t>(mesh_indices);
            newNode->box = BoundingBox3f(box);

            return newNode;
        }

        Node *parent = new Node();
        parent->box = BoundingBox3f(box);

        auto boxes = subdivideBox(parent->box);

        std::vector<std::vector<uint32_t>> lists(8);
        std::vector<std::vector<uint32_t>> mesh_indices_lists(8);
        for (uint32_t i = 0; i < triangle_indices.size(); i++) {
            uint32_t index = triangle_indices[i];
            uint32_t mesh_index = mesh_indices[i];

            auto triangleBox = m_meshes[mesh_index]->getBoundingBox(index);
            for (int j = 0; j < 8; j++) {
                if (boxes.at(j).overlaps(triangleBox)) {
                    lists[j].push_back(index);
                    mesh_indices_lists[j].push_back(mesh_index);
                }
            }
        }

        std::vector<Node*> nodes(8);
        tbb::blocked_range<int> range(0, 8);
        auto map = [&](const tbb::blocked_range<int> &range) {
            for (int i = range.begin(); i != range.end(); i++) {
                BoundingBox3f childBox = boxes.at(i);
                auto list = lists.at(i);
                auto mesh_indices_list = mesh_indices_lists.at(i);
                Node* result = build(childBox, list, mesh_indices_list, recursive_depth+1);

                if (result != nullptr)
                    nodes[i] = result;
            }
        };
        tbb::parallel_for(range, map);

        for (auto value : nodes) {
            if (value != nullptr)
                parent->children.push_back(value);
        }

        return parent;
    }

    void Accel::flattenTree()
    {
        std::queue<Node*> queue;
        queue.push(m_root);
        int index = 0;

        m_flattenedNodes.resize(1);

        while (!queue.empty()) {
            Node* current = queue.front();
            FlatNode& currentFlat = m_flattenedNodes.at(index);
            uint32_t childrenSize = current->children.size();

            currentFlat.box = current->box;
            currentFlat.mesh_indices = current->mesh_indices;
            currentFlat.triangle_indices = current->triangle_indices;
            currentFlat.childIndexStart = m_flattenedNodes.size();
            currentFlat.childSize = childrenSize;

            for (int i = 0; i < childrenSize; i++) {
                queue.push(current->children.at(i));
            }
            if (childrenSize > 0)
                m_flattenedNodes.resize(m_flattenedNodes.size() + current->children.size());

            queue.pop();
            index++;
        }


    }

    std::vector<BoundingBox3f> subdivideBox(BoundingBox3f &parent) {
        Point3f extents = parent.getExtents();

        std::vector<BoundingBox3f> bboxes(8);
        Point3f x0_y0_z0 = parent.min;
        Point3f x1_y0_z0 = Point3f(parent.min.x() + extents.x() / 2.f, parent.min.y(), parent.min.z());
        Point3f x0_y1_z0 = Point3f(parent.min.x(), parent.min.y() + extents.y() / 2.f, parent.min.z());
        Point3f x1_y1_z0 = Point3f(parent.min.x() + extents.x() / 2.f, parent.min.y() + extents.y() / 2.f, parent.min.z());

        Point3f x0_y0_z1 = Point3f(parent.min.x(), parent.min.y(), parent.min.z() + extents.z() / 2.f);
        Point3f x1_y0_z1 = Point3f(parent.min.x() + extents.x() / 2.f, parent.min.y(), parent.min.z() + extents.z() / 2.f);
        Point3f x0_y1_z1 = Point3f(parent.min.x(), parent.min.y() + extents.y() / 2.f, parent.min.z() + extents.z() / 2.f);
        Point3f x1_y1_z1 = Point3f(parent.min.x() + extents.x() / 2.f, parent.min.y() + extents.y() / 2.f, parent.min.z() + extents.z() / 2.f);
        Point3f x2_y1_z1 = Point3f(parent.max.x(), parent.min.y() + extents.y() / 2.f, parent.min.z() + extents.z() / 2.f);
        Point3f x1_y2_z1 = Point3f(parent.min.x() + extents.x() / 2.f, parent.max.y(), parent.min.z() + extents.z() / 2.f);
        Point3f x2_y2_z1 = Point3f(parent.max.x(), parent.max.y(), parent.min.z() + extents.z() / 2.f);

        Point3f x1_y1_z2 = Point3f(parent.min.x() + extents.x() / 2.f, parent.min.y() + extents.y() / 2.f, parent.max.z());
        Point3f x2_y1_z2 = Point3f(parent.max.x(), parent.min.y() + extents.y() / 2.f, parent.max.z());
        Point3f x1_y2_z2 = Point3f(parent.min.x() + extents.x() / 2.f, parent.max.y(), parent.max.z());
        Point3f x2_y2_z2 = Point3f(parent.max.x(), parent.max.y(), parent.max.z());

        bboxes[0] = BoundingBox3f(x0_y0_z0, x1_y1_z1);
        bboxes[1] = BoundingBox3f(x1_y0_z0, x2_y1_z1);
        bboxes[2] = BoundingBox3f(x0_y1_z0, x1_y2_z1);
        bboxes[3] = BoundingBox3f(x1_y1_z0, x2_y2_z1);
        bboxes[4] = BoundingBox3f(x0_y0_z1, x1_y1_z2);
        bboxes[5] = BoundingBox3f(x1_y0_z1, x2_y1_z2);
        bboxes[6] = BoundingBox3f(x0_y1_z1, x1_y2_z2);
        bboxes[7] = BoundingBox3f(x1_y1_z1, x2_y2_z2);

        return bboxes;
    }

LUMINA_NAMESPACE_END
