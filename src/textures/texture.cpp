#include "texture.h"

LUMINA_NAMESPACE_BEGIN

UVMapping2D::UVMapping2D(float su, float sv, float du, float dv)
	: su(su), sv(sv), du(du), dv(dv) {}

Point2f UVMapping2D::map(Intersection& its, Vector2f& dstdx, Vector2f& dstdy) const
{
	return Point2f(su * its.uv.x() + du, sv * its.uv.y() + dv);
}

Point3f TransformMapping3D::map(Intersection& its, Vector3f& dstdx, Vector3f& dstdy) const
{

	return worldToTexture * its.p;
}

float lanczos(float x, float tau)
{
    x = std::abs(x);
    if (x < 1e-5f) return 1;
    if (x > 1.f) return 0;
    x *= M_PI;
    float s = std::sin(x * tau) / (x * tau);
    float lanczos = std::sin(x) / x;
    return s * lanczos;
}

LUMINA_NAMESPACE_END

