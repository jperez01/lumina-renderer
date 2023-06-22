//
// Created by juperez on 5/25/23.
//

#pragma once

#include "core/object.h"
#include "primitives/mesh.h"

LUMINA_NAMESPACE_BEGIN

enum EmitterType {
    AREA_LIGHT
};

/**
 * Data struct for parameters used for an emitter
 */
struct EmitterQueryRecord {
    // Position from emitter mesh
    Point3f p;
    // Normal from emitter mesh
    Normal3f n;

    Vector3f oToP;
    // Direction from some mesh to the emitter
    Vector3f wi;

    // Point on mesh that wants to get its light contribution from this source
    Point3f refOrigin;
    // Normal on mesh
    Vector3f refNormal;

    // Pdf of emitter
    float pdf;

    EmitterQueryRecord(Point3f p) : refOrigin(p) {}
    EmitterQueryRecord(Point3f p, Vector3f n) : refOrigin(p), refNormal(n) {}
};

class Emitter : public LuminaObject {
public:
    virtual ~Emitter() {}

    virtual Color3f sample(EmitterQueryRecord& record, const Point2f& sample) const = 0;
    virtual float pdf() const = 0;
    virtual Color3f eval(const EmitterQueryRecord& record) const = 0;

    virtual Color3f getRadiance() const = 0;
    Mesh* getMesh() { return m_mesh; }
    void setMesh(Mesh* mesh) { m_mesh = mesh; }

    EClassType getClassType() const { return EEmitter; }

protected:
    Mesh* m_mesh;
    EmitterType m_type;
};

inline void convertToSolidAngle(EmitterQueryRecord& record) {
    // Float pdf = DistanceSquared(ref.p, isectLight.p) /
    //   (AbsDot(isectLight.n, -wi) * Area());

    Vector3f oToP = record.p - record.refOrigin;
    float distanceSquared = oToP.dot(oToP);
    float cosTheta = abs(record.n.dot(-record.wi));

    if (cosTheta == 0.0f) {
        record.pdf = 0.0f;
    }
    else {
        record.pdf = record.pdf * distanceSquared / cosTheta;
    }
}

LUMINA_NAMESPACE_END