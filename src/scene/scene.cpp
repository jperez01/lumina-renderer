//
// Created by juperez on 5/24/23.
//

#include "scene.h"

LUMINA_NAMESPACE_BEGIN

Scene::Scene(const PropertyList &) {
    m_accel = new Accel();
}

Scene::~Scene() {
    delete m_accel;
    delete m_sampler;
    delete m_camera;
    delete m_integrator;
}

void Scene::activate() {
    m_accel->build();

    if (!m_integrator)
        throw LuminaException("No integrator was specified.");
    if (!m_camera)
        throw LuminaException("No camera was specified.");

    if (!m_sampler) {
        m_sampler = dynamic_cast<Sampler*>(
                LuminaObjectFactory::createInstance("independent", PropertyList())
                );
    }

    std::cout << std::endl;
    std::cout << "Configuration: " << toString() << std::endl;
    std::cout << std::endl;
}

void Scene::addChild(LuminaObject *obj) {
    switch(obj->getClassType()) {
        case EMesh: {
            Mesh* mesh = dynamic_cast<Mesh*>(obj);
            m_accel->addMesh(mesh);
            m_meshes.push_back(mesh);
            if (mesh->isEmitter())
                m_emitters.push_back(mesh->getEmitter());
        }
        break;

        case EEmitter: {
            Emitter* emitter = dynamic_cast<Emitter*>(obj);
            m_emitters.push_back(emitter);
        }
        break;

        case ESampler: {
            if (m_sampler)
                throw LuminaException("Sampler has already been added.");
            m_sampler = dynamic_cast<Sampler*>(obj);
        }
        break;

        case EIntegrator: {
            if (m_integrator)
                throw LuminaException("Integrator has already been added.");
            m_integrator = dynamic_cast<Integrator*>(obj);
        }
        break;

        case ECamera: {
            if (m_camera)
                throw LuminaException("Camera has already been added.");
            m_camera = dynamic_cast<Camera*>(obj);
        }
        break;

        default:
            throw LuminaException("Scene::addChild(<%s>) is not supported!",
                                  classTypeName(obj->getClassType()));
    }
}

std::string Scene::toString() const {
    std::string meshes;
    for (size_t i=0; i<m_meshes.size(); ++i) {
        meshes += std::string("  ") + indent(m_meshes[i]->toString(), 2);
        if (i + 1 < m_meshes.size())
            meshes += ",";
        meshes += "\n";
    }

    return tfm::format(
            "Scene[\n"
            "  integrator = %s,\n"
            "  sampler = %s\n"
            "  camera = %s,\n"
            "  meshes = {\n"
            "  %s  }\n"
            "]",
            indent(m_integrator->toString()),
            indent(m_sampler->toString()),
            indent(m_camera->toString()),
            indent(meshes, 2)
    );
}

bool Scene::rayIntersect(const Ray3f &ray, Intersection &its) const {
    return m_accel->rayIntersect(ray, its, false);
}

bool Scene::rayIntersect(const Ray3f &ray) const {
    Intersection its;

    return m_accel->rayIntersect(ray, its, true);
}

Emitter *Scene::sampleLight(float sample, float& pdf) const {
    int index = (int) (sample * m_emitters.size());

    pdf = 1.0f / m_emitters.size();
    return m_emitters[index];
}

    LUMINA_REGISTER_CLASS(Scene, "scene")
LUMINA_NAMESPACE_END