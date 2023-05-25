//
// Created by juperez on 5/25/23.
//

#include "mesh.h"
#include <Eigen/Geometry>

LUMINA_NAMESPACE_BEGIN

Mesh::Mesh() { }

Mesh::~Mesh() {
    delete m_bsdf;
    delete m_emitter;
}

void Mesh::activate() {
    if (!m_bsdf) {
        m_bsdf = dynamic_cast<BSDF *>(
            LuminaObjectFactory::createInstance("diffuse", PropertyList())
            );
    }
}

float Mesh::surfaceArea(uint32_t index) const {
    uint32_t i0 = m_faces(0, index), i1 = m_faces(1, index), i2 = m_faces(2, index);

    Point3f p0 = m_vertices.col(i0), p1 = m_vertices.col(i1), p2 = m_vertices.col(i2);

    return 0.5f * Vector3f((p1 - p0).cross(p2 - p0)).norm();
}

bool Mesh::rayIntersect(uint32_t index, const Ray3f &ray, float &u, float &v) const {
    uint32_t i0 = m_faces(0, index), i1 = m_faces(1, index),
        i2 = m_faces(2, index);
    Point3f p0 = m_vertices.col(i0), p1 = m_vertices.col(i1), p2 = m_vertices.col(i2);

    Vector3f edge1 = p1 - p0, edge2 = p2 - p0;
    Vector3f pvec = ray.d.cross(edge2);

    float det = edge1.dot(pvec);
    if (det > -Epsilon && det < Epsilon)
        return false;
    float invDet = 1 / det;

    Vector3f tvec = ray.o - p0;

    //Calculate u param
    u = tvec.dot(pvec) * invDet;
    if (u < 0.0f || u > 1.0f)
        return false;

    Vector3f qvec = tvec.cross(edge1);

    // Calculate v param
    v = ray.d.dot(qvec) * invDet;
    if (v < 0.0f || v + u > 1.0f)
        return false;

    float t = edge2.dot(qvec) * invDet;

    return t >= ray.mint && t <= ray.maxt;
}

Point3f Mesh::getCentroid(uint32_t index) const {
    uint32_t i0 = m_faces(0, index), i1 = m_faces(1, index),
            i2 = m_faces(2, index);
    Point3f p0 = m_vertices.col(i0), p1 = m_vertices.col(i1), p2 = m_vertices.col(i2);

    return (p0 + p1 + p2) / 3.0f;
}

BoundingBox3f Mesh::getBoundingBox(uint32_t index) const {
    uint32_t i0 = m_faces(0, index), i1 = m_faces(1, index),
            i2 = m_faces(2, index);
    Point3f p0 = m_vertices.col(i0), p1 = m_vertices.col(i1), p2 = m_vertices.col(i2);

    BoundingBox3f box(p0);
    box.expandBy(p1);
    box.expandBy(p2);

    return box;
}

void Mesh::addChild(LuminaObject *child) {
    switch (child->getClassType()) {
        case EBSDF:
            if (m_bsdf)
                throw LuminaException("Already have a bsdf specified.");
            m_bsdf = dynamic_cast<BSDF *>(child);
            break;

        case EEmitter: {
            if (m_emitter)
                throw LuminaException("Already have an emitter specified.");

            m_emitter = dynamic_cast<Emitter *>(child);
            }
            break;
        default:
            throw LuminaException("Mesh::addChild(%s) is not supported",
                                  classTypeName(child->getClassType()));
    }
}

std::string Mesh::toString() const {
    return tfm::format(
            "Mesh[\n"
            "  name = \"%s\",\n"
            "  vertexCount = %i,\n"
            "  triangleCount = %i,\n"
            "  bsdf = %s,\n"
            "  emitter = %s\n"
            "]",
            m_name,
            m_vertices.cols(),
            m_faces.cols(),
            m_bsdf ? indent(m_bsdf->toString()) : std::string("null"),
            m_emitter ? indent(m_emitter->toString()) : std::string("null")
    );
}

std::string Intersection::toString() const {
    if (!mesh)
        return "Intersection[invalid]";

    return tfm::format(
            "Intersection[\n"
            "  p = %s,\n"
            "  t = %f,\n"
            "  uv = %s,\n"
            "  shFrame = %s,\n"
            "  geoFrame = %s,\n"
            "  mesh = %s\n"
            "]",
            p.toString(),
            t,
            uv.toString(),
            indent(shadingFrame.toString()),
            indent(geoFrame.toString()),
            mesh ? mesh->toString() : std::string("null")
    );
}

LUMINA_NAMESPACE_END