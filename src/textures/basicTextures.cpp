#include "texture.h"

LUMINA_NAMESPACE_BEGIN

template <typename T> class ConstantTexture : public Texture<T> {
public:
	ConstantTexture(const T& value) : value(value) {}
	T evaluate(Intersection& its) const { return value; }

private:
	T value;
};

template <typename T1, typename T2> class ScaleTexture : public Texture<T2> {
public:
	ScaleTexture(std::shared_ptr<Texture<T1>>& tex1, std::shared_ptr<Texture<T2>>& tex2) 
		: tex1(tex1), tex2(tex2) {}
	T2 evaluate(Intersection& its) const {
		return tex1->evaluate(its) * tex2->evaluate(its);
	}

private:
	std::shared_ptr<Texture<T1>> tex1;
	std::shared_ptr<Texture<T2>> tex2;
};

template <typename T> class MixTexture : public Texture<T> {
public:
	MixTexture(std::shared_ptr<Texture<T>>& tex1, std::shared_ptr<Texture<T>>& tex2,
		std::shared_ptr<Texture<float>>& amount)
		: tex1(tex1), tex2(tex2), amount(amount) {}
	T evaluate(Intersection& its) const {
		T t1 = tex1->evaluate(its), t2 = tex2->evaluate(its);
		float someAmount = amount->evaluate(its);

		return (1.0f - someAmount) * t1 + someAmount * t2;
	}

private:
	std::shared_ptr<Texture<T>> tex1, tex2;
	std::shared_ptr<Texture<float>> amount;
};

template <typename T> class BilerpTexture : public Texture<T> {
public:
	BilerpTexture(std::unique_ptr<TextureMapping2D> mapping, const T & v00,
		const T & v01, const T & v10, const T & v11)
	: mapping(std::move(mapping)), v00(v00), v01(v01), v10(v10), v11(v11) { }

	T Evaluate(Intersection& its) const {
		Vector2f dstdx, dstdy;
		Point2f st = mapping->map(its, &dstdx, &dstdy);
		return (1 - st[0]) * (1 - st[1]) * v00 + (1 - st[0]) * (st[1]) * v01 +
			(st[0]) * (1 - st[1]) * v10 + (st[0]) * (st[1]) * v11;
	}

private:
	std::unique_ptr<TextureMapping2D> mapping;
	const T v00, v01, v10, v11;

};

LUMINA_NAMESPACE_END