//
// Created by juperez on 5/24/23.
//

#pragma once
#include "common.h"
#include "propsList.h"
#include <map>

LUMINA_NAMESPACE_BEGIN

class LuminaObject {
public:
    enum EClassType {
        EScene = 0,
        EMesh,
        ECamera,
        EBSDF,
        EPhaseFunction,
        ESampler,
        EMedium,
        EIntegrator,
        EEmitter,
        ETest,
        EReconstructionFilter,
        ETexture,
        EFloatTexture, EColorTexture,
        EClassTypeCount
    };

    virtual ~LuminaObject() {}

    virtual EClassType getClassType() const = 0;
    virtual EClassType getTemplatedClassType() const { return ETexture; }

    virtual void addChild(LuminaObject* child);
    virtual void setParent(LuminaObject* parent);

    virtual void activate();
    virtual std::string toString() const {
        return "Lumina Object";
    }

    static std::string classTypeName(EClassType type) {
        switch (type) {
            case EScene: return "scene";
            case EMesh: return "mesh";
            case ECamera: return "camera";
            case EIntegrator: return "integrator";
            case EBSDF: return "bsdf";
            case ESampler: return "sampler";
            case ETest: return "test";
            case EEmitter: return "emitter";
            case EReconstructionFilter: return "reconstruction filter";
            case ETexture: return "texture";
            case EClassTypeCount: return "class type count";
            default: return "<unknown>";
        }
    }
};

class LuminaObjectFactory {
public:
    typedef std::function<LuminaObject* (const PropertyList&)> Constructor;

    static void registerClass(const std::string& name, const Constructor& constructor);

    static LuminaObject* createInstance(const std::string& name, const PropertyList &props) {
        if (!m_constructors || m_constructors->find(name) == m_constructors->end())
            throw LuminaException("A constructor for class %s could not be found.", name);

        return (*m_constructors)[name](props);
    }
private:
    static std::map<std::string, Constructor> *m_constructors;
};

template<typename T>
LuminaObjectFactory::Constructor createConstructor() {
    return [](const PropertyList& list) -> T* {
        return new T(list);
    };
}

#define LUMINA_REGISTER_CLASS(cls, name) \
    cls *cls ##_create(const PropertyList &list) { \
        return new cls(list); \
    } \
    static struct cls ##_{ \
        cls ##_() { \
            LuminaObjectFactory::registerClass(name, cls ##_create); \
        } \
    } cls ##__LUMINA_;

#define LUMINA_TEMPLATED_REGISTER_CLASS(templatecls, cls, name) \
    templatecls *cls ##_create(const PropertyList &list) { \
        return new templatecls(list); \
    } \
    static struct cls ##_{ \
        cls ##_() { \
            LuminaObjectFactory::registerClass(name, cls ##_create); \
        } \
    } cls ##__LUMINA_;

LUMINA_NAMESPACE_END