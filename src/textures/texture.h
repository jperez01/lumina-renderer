#pragma once

#include "core/common.h"
#include "primitives/mesh.h"

LUMINA_NAMESPACE_BEGIN

enum TextureType {
	Default,
	Albedo,
	Normal,
	Metallic,
	Roughness,
	AO
};

class TextureMapping2D {
public:
	virtual ~TextureMapping2D() {}
	virtual Point2f map(Intersection& its, Vector2f& dstdx, Vector2f& dstdy) const = 0;
};

class UVMapping2D : public TextureMapping2D {
public:
	UVMapping2D(float su = 1, float sv = 1, float du = 0, float dv = 0);
	Point2f map(Intersection& its, Vector2f& dstdx, Vector2f& dstdy) const;
private:
	const float su, sv, du, dv;
};

class TextureMapping3D {
public:
	virtual ~TextureMapping3D() {}
	virtual Point3f map(Intersection& its, Vector3f& dstdx, Vector3f& dstdy) const = 0;
};

class TransformMapping3D : TextureMapping3D {
public:
	TransformMapping3D(const Transform worldToTexture) : worldToTexture(worldToTexture) {}
	Point3f map(Intersection& its, Vector3f& dstdx, Vector3f& dstdy) const;
private:
	const Transform worldToTexture;
};

template <typename T> class Texture : public LuminaObject {
public:
	virtual T evaluate(Intersection& its) const = 0;
	virtual ~Texture() {}

	virtual EClassType getClassType() const { return ETexture; }
	TextureType getTextureType() const { return type; }

protected:
	TextureType type = Default;
};

float lanczos(float x, float tau = 2);

LUMINA_NAMESPACE_END