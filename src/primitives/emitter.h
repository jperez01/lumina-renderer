//
// Created by juperez on 5/25/23.
//

#pragma once

#include "core/object.h"
#include "mesh.h"

LUMINA_NAMESPACE_BEGIN

class Emitter : public LuminaObject {
public:
    virtual void setMesh(Mesh* mesh) = 0;

    virtual Color3f sample();

    EClassType getClassType() const { return EEmitter; }
protected:
};

LUMINA_NAMESPACE_END