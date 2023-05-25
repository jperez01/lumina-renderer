//
// Created by juperez on 5/25/23.
//

#pragma once

#include "primitives/frame.h"
#include "core/object.h"
#include "bbox.h"
#include "emitter.h"
#include "bsdfs/bsdf.h"

LUMINA_NAMESPACE_BEGIN

class Mesh;

struct Intersection {
    /// Position of intersection
    Point3f p;
    /// Distance along the ray
    float t;
    /// UV coordinates
    Point2f uv;
    /// Shading Frame (based on normal)
    Frame shadingFrame;
    /// Geometric frame (based on the actual geometry)
    Frame geoFrame;
    /// Pointer to associated mesh
    const Mesh* mesh;

    Intersection() : mesh(nullptr) {}

    Vector3f toLocal(const Vector3f& d) const {
        return shadingFrame.toLocal(d);
    }

    Vector3f toWorld(const Vector3f& d) const {
        return shadingFrame.toWorld(d);
    }

    std::string toString() const;
};

class Mesh : public LuminaObject {
public:
    virtual ~Mesh();

    virtual void activate();

    /// Return the total number of triangles in this shape
    uint32_t getTriangleCount() const { return (uint32_t) m_faces.cols(); }

    /// Return the total number of vertices in this shape
    uint32_t getVertexCount() const { return (uint32_t) m_vertices.cols(); }

    /// Return the surface area of the given triangle
    float surfaceArea(uint32_t index) const;

    //// Return an axis-aligned bounding box of the entire mesh
    const BoundingBox3f &getBoundingBox() const { return m_bbox; }

    //// Return an axis-aligned bounding box containing the given triangle
    BoundingBox3f getBoundingBox(uint32_t index) const;

    //// Return the centroid of the given triangle
    Point3f getCentroid(uint32_t index) const;

    bool rayIntersect(uint32_t index, const Ray3f& ray, float& u, float& v) const;

    /// Return a pointer to the vertex positions
    const MatrixXf &getVertexPositions() const { return m_vertices; }

    /// Return a pointer to the vertex normals (or \c nullptr if there are none)
    const MatrixXf &getVertexNormals() const { return m_normals; }

    /// Return a pointer to the texture coordinates (or \c nullptr if there are none)
    const MatrixXf &getVertexTexCoords() const { return m_uvs; }

    /// Return a pointer to the triangle vertex index list
    const MatrixXu &getIndices() const { return m_faces; }

    /// Is this mesh an area emitter?
    bool isEmitter() const { return m_emitter != nullptr; }

    /// Return a pointer to an attached area emitter instance
    Emitter *getEmitter() { return m_emitter; }

    /// Return a pointer to an attached area emitter instance (const version)
    Emitter *getEmitter() const { return m_emitter; }

    /// Return a pointer to the BSDF associated with this mesh
    const BSDF *getBSDF() const { return m_bsdf; }

    /// Register a child object (e.g. a BSDF) with the mesh
    virtual void addChild(LuminaObject *child);

    /// Return the name of this mesh
    const std::string &getName() const { return m_name; }

    /// Return a human-readable summary of this instance
    std::string toString() const;

    /**
     * \brief Return the type of object (i.e. Mesh/BSDF/etc.)
     * provided by this instance
     * */
    EClassType getClassType() const { return EMesh; }

protected:
    Mesh();

    std::string m_name;
    MatrixXf m_vertices;
    MatrixXf m_normals;
    MatrixXf m_uvs;
    MatrixXu m_faces;

    BSDF* m_bsdf = nullptr;
    Emitter* m_emitter = nullptr;
    BoundingBox3f m_bbox;
};

LUMINA_NAMESPACE_END
