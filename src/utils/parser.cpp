//
// Created by juperez on 5/24/23.
//

#include "parser.h"
#include "Eigen/Geometry"

#include <fstream>

LUMINA_NAMESPACE_BEGIN

std::string offset(std::string& filename, ptrdiff_t pos) {
    std::fstream is(filename);

    char buffer[1024];
    int line = 0, linestart = 0, offset = 0;
    while (is.good()) {
        is.read(buffer, sizeof(buffer));
        for (int i = 0; i < is.gcount(); i++) {
            if (buffer[i] == '\n') {
                if (offset + i >= pos)
                    return tfm::format("line %i, col %i", line+1, pos - linestart);
                line++;
                linestart = offset = i;
            }
        }
        offset += (int) is.gcount();
    }

    return "byte offset " + std::to_string(pos);
}

// Helper function that checks if attributes are fully specified
void check_attributes(std::string& filename, const pugi::xml_node& node, std::set<std::string> attrs) {
    for (auto attr : node.attributes()) {
        auto it = attrs.find(attr.name());
        if (it == attrs.end())
            throw LuminaException("Error while parsing \"%s\": unexpected attribute \"%s\" in \"%s\" at %s",
                  filename, attr.name(), node.name(), offset(filename, node.offset_debug()));

        attrs.erase(it);
    }

    if (!attrs.empty())
        throw LuminaException("Error while parsing \"%s\": missing attribute \"%s\" in \"%s\" at %s",
              filename, *attrs.begin(), node.name(), offset(filename, node.offset_debug()));
}

LuminaObject* loadXMLFile(std::string &filename) {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(filename.c_str());
    if (!result)
        throw LuminaException("Error while parsing %s: %s (at %s)",
              filename, result.description(), offset(filename, result.offset));

    enum ETag {
        EScene = LuminaObject::EScene,
        EMesh = LuminaObject::EMesh,
        EBSDF = LuminaObject::EBSDF,
        EPhaseFunction = LuminaObject::EPhaseFunction,
        EEmitter = LuminaObject::EEmitter,
        EMedium = LuminaObject::EMedium,
        ECamera = LuminaObject::ECamera,
        EIntegrator = LuminaObject::EIntegrator,
        ESampler = LuminaObject::ESampler,
        ETest = LuminaObject::ETest,
        EReconstructionFilter = LuminaObject::EReconstructionFilter,

        EBoolean = LuminaObject::EClassTypeCount,
        EInteger,
        EFloat,
        EString,
        EPoint,
        EVector,
        EColor,
        ETransform,
        ETranslate,
        EMatrix,
        ERotate,
        EScale,
        ELookAt,

        EInvalid
    };

    std::map<std::string, ETag> tags;
    tags["scene"]      = EScene;
    tags["mesh"]       = EMesh;
    tags["bsdf"]       = EBSDF;
    tags["emitter"]  = EEmitter;
    tags["camera"]     = ECamera;
    tags["medium"]     = EMedium;
    tags["phase"]      = EPhaseFunction;
    tags["integrator"] = EIntegrator;
    tags["sampler"]    = ESampler;
    tags["rfilter"]    = EReconstructionFilter;
    tags["test"]       = ETest;
    tags["boolean"]    = EBoolean;
    tags["integer"]    = EInteger;
    tags["float"]      = EFloat;
    tags["string"]     = EString;
    tags["point"]      = EPoint;
    tags["vector"]     = EVector;
    tags["color"]      = EColor;
    tags["transform"]  = ETransform;
    tags["translate"]  = ETranslate;
    tags["matrix"]     = EMatrix;
    tags["rotate"]     = ERotate;
    tags["scale"]      = EScale;
    tags["lookat"]     = ELookAt;

    Eigen::Affine3f transform;

    std::function<LuminaObject*(pugi::xml_node &, PropertyList &, int)> parseTag =
            [&](pugi::xml_node& node, PropertyList& list, int parentTag) -> LuminaObject*
    {
        if (node.type() == pugi::node_comment || node.type() == pugi::node_declaration)
            return nullptr;

        if (node.type() != pugi::node_element)
            throw LuminaException(
                    "Error while parsing \"%s\": unexpected content at %s",
                    filename, offset(filename, node.offset_debug()));

        auto it = tags.find(node.name());
        if (it == tags.end())
            throw LuminaException(
                    "Error while parsing \"%s\": unexpected tag \"%s\" at %s",
                    filename, node.name(), offset(filename, node.offset_debug()));

        int tag = it->second;

        bool hasParent = parentTag != EInvalid;
        bool parentIsObject = hasParent && parentTag < LuminaObject::EClassTypeCount;
        bool currentIsObject = tag < LuminaObject::EClassTypeCount;
        bool parentIsTransform = parentTag == ETransform;
        bool currentIsTransformOp = tag == ETranslate || tag == ERotate || tag == EScale || tag == ELookAt || tag == EMatrix;

        if (!hasParent && !currentIsObject)
            throw LuminaException("Error while parsing \"%s\": root element \"%s\" must be a Nori object (at %s)",
                                filename, node.name(), offset(filename, node.offset_debug()));

        if (parentIsTransform != currentIsTransformOp)
            throw LuminaException("Error while parsing \"%s\": transform nodes "
                                "can only contain transform operations (at %s)",
                                filename,  offset(filename, node.offset_debug()));

        if (hasParent && !parentIsObject && !(parentIsTransform && currentIsTransformOp))
            throw LuminaException("Error while parsing \"%s\": node \"%s\" requires a Nori object as parent (at %s)",
                                filename, node.name(), offset(filename, node.offset_debug()));

        if (tag == EScene)
            node.append_attribute("type") = "scene";
        else if (tag == ETransform)
            transform.setIdentity();

        PropertyList propList;
        std::vector<LuminaObject*> children;

        for (pugi::xml_node& child : node.children()) {
            LuminaObject* childObject = parseTag(child, propList, tag);
            if (childObject)
                children.push_back(childObject);
        }

        LuminaObject* result = nullptr;
        try {
            if (currentIsObject) {
                check_attributes(filename, node, {"type"});

                result = LuminaObjectFactory::createInstance(node.attribute("type").value(),
                                                             propList);

                if (result->getClassType() != (int) tag) {
                    throw LuminaException(
                            "Unexpectedly constructed an object "
                            "of type <%s> (expected type <%s>): %s",
                            LuminaObject::classTypeName(result->getClassType()),
                            LuminaObject::classTypeName((LuminaObject::EClassType) tag),
                            result->toString()
                            );
                }

                for (auto child : children) {
                    result->addChild(child);
                    child->setParent(result);
                }

                result->activate();
            } else {
                switch (tag) {
                    case EString: {
                            check_attributes(filename, node, {"name", "value"});
                            list.setString(node.attribute("name").value(), node.attribute("value").value());
                        }
                        break;
                    case EFloat: {
                            check_attributes(filename, node, {"name", "value"});
                            list.setFloat(node.attribute("name").value(), toFloat(node.attribute("value").value()));
                        }
                        break;
                    case EInteger: {
                            check_attributes(filename, node, {"name", "value"});
                            list.setInteger(node.attribute("name").value(), toInt(node.attribute("value").value()));
                        }
                        break;
                    case EBoolean: {
                            check_attributes(filename, node, {"name", "value"});
                            list.setBoolean(node.attribute("name").value(), toBool(node.attribute("value").value()));
                        }
                        break;
                    case EPoint: {
                            check_attributes(filename, node, {"name", "value"});
                            Point3f point(toVector3f(node.attribute("value").value()));
                            list.setPoint(node.attribute("name").value(), point);
                        }
                        break;
                    case EVector: {
                            check_attributes(filename, node, {"name", "value"});
                            Vector3f vector(toVector3f(node.attribute("value").value()));
                            list.setVector(node.attribute("name").value(), vector);
                        }
                        break;
                    case EColor: {
                            check_attributes(filename, node, {"name", "value"});
                            Color3f color(toVector3f(node.attribute("value").value()).array());
                            list.setColor(node.attribute("name").value(), color);
                        }
                        break;
                    case ETransform: {
                            check_attributes(filename, node, {"name"});
                            list.setTransform(node.attribute("name").value(), transform.matrix());
                        }
                        break;
                    case ETranslate: {
                            check_attributes(filename, node, {"value"});
                            Eigen::Vector3f vector(toVector3f(node.attribute("value").value()));
                            transform = Eigen::Translation<float, 3>(vector) * transform;
                        }
                        break;
                    case EMatrix: {
                            check_attributes(filename, node, {"value"});

                            auto tokens = tokenize(node.attribute("value").value());
                            if (tokens.size() != 16)
                                throw LuminaException("Expected 16 values");

                            Eigen::Matrix4f matrix;
                            for (int i = 0; i < 4; i++) {
                                for (int j = 0; j < 4; j++) {
                                    matrix(i, j) = toFloat(tokens[i * 4 + j]);
                                }
                            }

                            transform = Eigen::Affine3f(matrix) * transform;
                        }
                        break;
                    case EScale: {
                            check_attributes(filename, node, {"value"});
                            Eigen::Vector3f scale(toVector3f(node.attribute("value").value()));

                            transform = Eigen::DiagonalMatrix<float, 3>(scale) * transform;
                        }
                        break;
                    case ERotate: {
                            check_attributes(filename, node, {"angle", "axis"});
                            float angle = degToRad(toFloat(node.attribute("angle").value()));
                            Eigen::Vector3f axis(toVector3f(node.attribute("axis").value()));

                            transform = Eigen::AngleAxis<float>(angle, axis) * transform;
                        }
                        break;
                    case ELookAt: {
                            check_attributes(filename, node, {"origin", "target", "up"});
                            Eigen::Vector3f origin(toVector3f(node.attribute("origin").value()));
                            Eigen::Vector3f target(toVector3f(node.attribute("target").value()));
                            Eigen::Vector3f up(toVector3f(node.attribute("up").value()));

                            Vector3f dir = (target - origin).normalized();
                            Vector3f left = up.normalized().cross(dir).normalized();
                            Vector3f newUp = dir.cross(left).normalized();

                            Eigen::Matrix4f newTransform;
                            newTransform << left, newUp, dir, origin,
                                0, 0, 0, 1;

                            transform = Eigen::Affine3f(newTransform) * transform;
                        }
                        break;
                    default: throw LuminaException("Unhandled element \"%s\"", node.name());
                }
            }
        } catch (const LuminaException& e) {
            throw LuminaException("Error while parsing \"%s\": %s (at %s)", filename,
                                  e.what(), offset(filename, node.offset_debug()));
        }

        return result;
    };
    PropertyList list;
    return parseTag(*doc.begin(), list, EInvalid);
}

LUMINA_NAMESPACE_END