//
// Created by juperez on 5/24/23.
//

#include "object.h"

LUMINA_NAMESPACE_BEGIN

void LuminaObject::addChild(lumina::LuminaObject *child) {
    throw LuminaException(
            "LuminaObject::addChild() is not implemented for objects of type '%s'!",
            classTypeName(getClassType())
            );
}

void LuminaObject::activate() {}
void LuminaObject::setParent(lumina::LuminaObject *parent) {}

std::map<std::string, LuminaObjectFactory::Constructor>* LuminaObjectFactory::m_constructors = nullptr;

void LuminaObjectFactory::registerClass(const std::string &name,
                                        const LuminaObjectFactory::Constructor &constructor) {
    if (!m_constructors)
        m_constructors = new std::map<std::string, LuminaObjectFactory::Constructor>();

    (*m_constructors)[name] = constructor;
}

LUMINA_NAMESPACE_END