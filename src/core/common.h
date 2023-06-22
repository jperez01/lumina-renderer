//
// Created by juperez on 5/23/23.
//

#pragma once

#include <iostream>
#include <algorithm>
#include <vector>
#include <nanogui/button.h>
#include <OpenEXRCore/openexr_version.h>
#include <tbb/task.h>
#include <Eigen/Core>
#include <tinyformat/tinyformat.h>


#include "utils/resolver.h"

#define LUMINA_NAMESPACE_BEGIN namespace lumina {
#define LUMINA_NAMESPACE_END }

// Useful Constants
#define Epsilon 1e-4f
#define Infinity std::numeric_limits<float>::infinity()

#undef M_PI

#define M_PI         3.14159265358979323846f
#define INV_PI       0.31830988618379067154f
#define INV_TWOPI    0.15915494309189533577f
#define INV_FOURPI   0.07957747154594766788f
#define SQRT_TWO     1.41421356237309504880f
#define INV_SQRT_TWO 0.70710678118654752440f

LUMINA_NAMESPACE_BEGIN
template <typename Scalar, int Dimension> struct TVector;
template <typename Scalar, int Dimension> struct TPoint;
template <typename Point, typename Vector> struct TRay;
template <typename Point> struct TBoundingBox;

typedef TVector<float, 1>       Vector1f;
typedef TVector<float, 2>       Vector2f;
typedef TVector<float, 3>       Vector3f;
typedef TVector<float, 4>       Vector4f;
typedef TVector<double, 1>       Vector1d;
typedef TVector<double, 2>       Vector2d;
typedef TVector<double, 3>       Vector3d;
typedef TVector<double, 4>       Vector4d;
typedef TVector<int, 1>       Vector1i;
typedef TVector<int, 2>       Vector2i;
typedef TVector<int, 3>       Vector3i;
typedef TVector<int, 4>       Vector4i;

typedef TPoint<float, 1>        Point1f;
typedef TPoint<float, 2>        Point2f;
typedef TPoint<float, 3>        Point3f;
typedef TPoint<float, 4>        Point4f;
typedef TPoint<double, 1>       Point1d;
typedef TPoint<double, 2>       Point2d;
typedef TPoint<double, 3>       Point3d;
typedef TPoint<double, 4>       Point4d;
typedef TPoint<int, 1>          Point1i;
typedef TPoint<int, 2>          Point2i;
typedef TPoint<int, 3>          Point3i;
typedef TPoint<int, 4>          Point4i;

typedef TBoundingBox<Point1f>   BoundingBox1f;
typedef TBoundingBox<Point2f>   BoundingBox2f;
typedef TBoundingBox<Point3f>   BoundingBox3f;
typedef TBoundingBox<Point4f>   BoundingBox4f;
typedef TBoundingBox<Point1d>   BoundingBox1d;
typedef TBoundingBox<Point2d>   BoundingBox2d;
typedef TBoundingBox<Point3d>   BoundingBox3d;
typedef TBoundingBox<Point4d>   BoundingBox4d;
typedef TBoundingBox<Point1i>   BoundingBox1i;
typedef TBoundingBox<Point2i>   BoundingBox2i;
typedef TBoundingBox<Point3i>   BoundingBox3i;
typedef TBoundingBox<Point4i>   BoundingBox4i;

typedef TRay<Point2f, Vector2f> Ray2f;
typedef TRay<Point3f, Vector3f> Ray3f;

class BSDF;
class Bitmap;
class BlockGenerator;
class Camera;
class ImageBlock;
class Integrator;
class KDTree;
class Emitter;
struct EmitterQueryRecord;
class Mesh;
class LuminaObject;
class LuminaObjectFactory;
class LuminaScreen;
class PhaseFunction;
class ReconstructionFilter;
class Sampler;
class Scene;

typedef Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> MatrixXf;
typedef Eigen::Matrix<uint32_t, Eigen::Dynamic, Eigen::Dynamic> MatrixXu;

enum EMeasure {
    EUnknownMeasure = 0,
    ESolidAngle,
    EDiscrete
};

class LuminaException : public std::runtime_error {
public:
    template <typename... Args> LuminaException(const char* fmt, const Args&... args)
    : std::runtime_error(tfm::format(fmt, args...)) {}
};

bool toBool(const std::string &str);
int toInt(const std::string& str);
unsigned int toUInt(const std::string& str);
float toFloat(const std::string& str);
Eigen::Vector3f toVector3f(const std::string& str);

std::string indent(const std::string &string, int amount = 2);

std::string timeString(double time, bool precise = false);

std::string memString(size_t size, bool precise = false);

Resolver* getFileResolver();

std::vector<std::string> tokenize(const std::string& s, const std::string& delim = ", ", bool includeEmpty = false);

inline float radToDeg(float value) { return value * (180.0f / M_PI); }
inline float degToRad(float value) { return value * (M_PI / 180.0f); }

std::string toLower(const std::string &value);

bool endsWith(const std::string &value, const std::string &ending);
float fresnel(float cosThetaI, float extIOR, float intIOR);

inline float gammaCorrect(float value) {
    if (value <= 0.0031308f)
        return 12.92f * value;
    else
        return 1.055f * std::pow(value, 1.0f/2.4f) - 0.055f;
}

inline float inverseGammaCorrect(float value) {
    if (value <= 0.04045f)
        return value * 1.0f / 12.92f;
    else
        return std::pow((value + 0.055f) * 1.0f / 1.055f, 2.4f);
}

inline float lerp(float t, float v1, float v2) { return (1 - t) * v1 + t * v2; }

template <typename T>
inline bool isPower2(T v) {
    return v && !(v & (v - 1));
}

inline int32_t roundUpPow2(int32_t v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    return v + 1;
}

inline int64_t roundUpPow2(int64_t v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v |= v >> 32;
    return v + 1;
}

LUMINA_NAMESPACE_END


