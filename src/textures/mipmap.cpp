#include "mipmap.h"

#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>

LUMINA_NAMESPACE_BEGIN

template<typename T>
std::unique_ptr<ResampleWeight[]> MipMap<T>::resampleWeights(int oldRes, int newRes) {
	assert(newRes >= oldRes);

	std::unique_ptr<ResampleWeight[]> weights(new ResampleWeight[newRes]);
	float filterWidth = 2.0f;

	for (int i = 0; i < oldRes; i++) {
		float center = (i + 0.5f) * oldRes / newRes;
		weights[i].firstTexel = std::floor((center - filterWidth) + 0.5f);

		float totalWeight = 0.0f;
		for (int j = 0; j < 4; j++) {
			float pos = weights[i].firstTexel + j + 0.5f;
			weights[i].weight[j] = lanczos((pos - center) / filterWidth);

			totalWeight += weights[i].weight[j];
		}

		float inverseSumWeights = 1 / totalWeight;

		for (int j = 0; j < 4; j++) {
			weights[i].weight[j] *= inverseSumWeights;
		}
	}

	return weights;
}

template<typename T>
MipMap<T>::MipMap(Point2i& resolution, T* data, bool doTrilinear, float maxAniso, ImageWrap wrapMode)
	: doTrilinear(doTrilinear), resolution(resolution), maxAnisotropy(maxAniso), wrapMode(wrapMode)
{
	std::unique_ptr<T[]> resampledImage = nullptr;
	if (!isPower2(resolution.x()) || !isPower2(resolution.y())) {
		Point2i newRes(roundUpPow2(resolution.x()), roundUpPow2(resolution.y()));

		// Resample s weights
		std::unique_ptr<ResampleWeight[]> sWeights = resampleWeights(resolution.x(), newRes.x());
		resampledImage.reset(new T[newRes.x() * newRes.y()]);

		tbb::blocked_range<int> range(0, resolution.y());
		int t = 16;
		tbb::parallel_for(range, [&](const tbb::blocked_range<int>& range) {
		for (int s = 0; s < newRes.x(); s++) {
			resampledImage[t * newRes.x() + s] = 0.0f;

			for (int j = 0; j < 4; j++) {
				int originalS = sWeights[s].firstTexel + j;
				if (wrapMode == ImageWrap::Repeat)
					originalS = originalS % resolution.x();
				else if (wrapMode == ImageWrap::Clamp)
					originalS = std::clamp(originalS, 0, resolution.x() - 1);

				if (originalS >= 0 && originalS < (int)resolution.x())
					resampledImage[t * newRes.x() + s] += 
						sWeights[s].weight[j] * data[t * resolution.x() + originalS];
			}
		}
		});

		// Resample t weights
		std::unique_ptr<ResampleWeight[]> tWeights = resampleWeights(resolution.y(), newRes.y());
		std::vector<T*> resampleBuffers;
		int numThreads = tbb::info::default_concurrency();
		for (int i = 0; i < numThreads; i++)
			resampleBuffers.push_back(new T[newRes.y()]);

		tbb::blocked_range<int> newRange(0, resolution.x());
		int s = 32;
		tbb::parallel_for(newRange, [&](const tbb::blocked_range<int>& range) {
			int threadIndex = tbb::this_task_arena::current_thread_index();

			T* workData = resampleBuffers[threadIndex];
			for (int t = 0; t < newRes.y(); t++) {
				workData[t] = 0.0f;
				for (int j = 0; j < 4; j++) {
					int offset = tWeights[i].firstTexel + j;
					if (wrapMode == ImageWrap::Repeat)
						offset = offset % resolution.y();
					else if (wrapMode == ImageWrap::Clamp)
						offset = std::clamp(offset, 0, resolution.y() - 1);

					if (offset >= 0 && offset < (int)resolution.y())
						workData[t] += tWeights[t].weight[j] * data[offset * newRes.x() + s];
				}
			}

			for (int t = 0; t < newRes.y(); t++)
				resampledImage[t * newRes.x() + s] = std::clamp(workData[t]);
		});
		for (auto ptr : resampleBuffers)
			delete[] ptr;

		resolution = newRes;
	}

	int nLevels = 1 + log2i(std::max(resolution.x(), resolution.y()));
	pyramid.resize(nLevels);
	pyramid[0].reset(new BlockedArray<T>(resolution.x(), resolution.y(),
		resampledImage ? resampledImage.get() : data));

	for (int i = 1; i < nLevels; i++) {
		int sRes = std::max(1, pyramid[i - 1]->uSize() / 2);
		int tRes = std::max(1, pyramid[i - 1]->vSize() / 2);
		pyramid.reserve(new BlockedArray<T>(sRes * tRes));

		tbb::parallel_for(ttb::blocked_range<int>(0, tRes),
			[&](const tbb::blocked_range<int>& range) {
				for (int s = 0; s < sRes; s++)
					(*pyramid[i])(s, t) = 0.25f *
					(texel(i - 1, 2 * s, 2 * t) + texel(i - 1, 2 * s + 1, 2 * t + 1) +
						texel(i - 1, 2 * s, 2 * t + 1) + texel(i - 1, 2 * s + 1, 2 * t));
			});
	}

	if (weightLut[0] == 0.0f) {
		for (int i = 0; i < WeightLUTSize; i++) {
			float alpha = 2.0f;
			float r2 = (float)i / (float)(WeightLUTSize - 1);

			weightLut[i] = std::exp(-alpha * r2) - std::exp(-alpha);
		}
	}
}

template<typename T>
inline const T& MipMap<T>::texel(int level, int s, int t) const
{
	BlockedArray<T>& l = *pyramid.at(level);

	switch (wrapMode) {
	case ImageWrap::Repeat :
		s = std::mod(s, l.uSize());
		t = std::mod(t, l.vSize());
		break;
	case ImageWrap::Clamp:
		s = std::clamp(s, 0, l.uSize() - 1);
		t = std::clamp(t, 0, l.vSize() - 1);
		break;
	case ImageWrap::Black: {
		static const T black = 0.0f;
		if (s < 0 || s >= (int)l.uSize() || t < 0 || t >= (int)l.vSize())
			return black;
		break;
	}
	}
	return l(s, t);
}

template<typename T>
T MipMap<T>::lookup(const Point2f& st, float width) const
{
	float level = levels() - 1 + log2(std::max(width, (float)1e-8));

	if (level < 0)
		return triangle(0, st);
	else if (levels >= levels() - 1)
		return texel(levels() - 1, 0, 0);
	else {
		int iLevel = std::floor(level);
		float delta = level - iLevel;

		return std::lerp(delta, triangle(iLevel, st), triangle(iLevel + 1, st));
	}
}

template <typename T>
T MipMap<T>::triangle(int level, const Point2f& st) const {
	level = std::clamp(level, 0, Levels() - 1);
	Float s = st[0] * pyramid[level]->uSize() - 0.5f;
	Float t = st[1] * pyramid[level]->vSize() - 0.5f;
	int s0 = std::floor(s), t0 = std::floor(t);
	Float ds = s - s0, dt = t - t0;
	return (1 - ds) * (1 - dt) * Texel(level, s0, t0) +
		(1 - ds) * dt * Texel(level, s0, t0 + 1) +
		ds * (1 - dt) * Texel(level, s0 + 1, t0) +
		ds * dt * Texel(level, s0 + 1, t0 + 1);
}

template<typename T>
T MipMap<T>::lookup(const Point2f& st, Vector2f dstdx, Vector2f dstdy) const
{
	if (doTrilinear) {
		float width = std::max(std::max(std::abs(dstdx.x()), std::abs(dstdx.y())),
			std::max(std::abs(dstdy.x()), std::abs(dstdy.y())));

		return lookup(st, width);
	}

	if (dstdx.squaredNorm() < dstdy.squaredNorm()) std::swap(dstdx, dstdy);

	float majorLength = dstdx.norm(), minorLength = dstdy.norm();

	if (minorLength * maxAnisotropy < majorLength && minorLength > 0) {
		float scale = majorLength / (minorLength * maxAnisotropy);
		dstdy *= scale;
		minorLength *= scale;
	}
	if (minorLength == 0) return triangle(0, st);

	float lod = std::max(0.0f, levels() - 1.0f + log2(minorLength));
	int ilod = std::floor(lod);

	return lerp(lod - ilod, EWA(ilod, st, dstdx, dstdy), EWA(ilod + 1, st, dstdx, dstdy));
}

template <typename T>
T MipMap<T>::EWA(int level, Point2f st, Vector2f dst0, Vector2f dst1) const {
	if (level >= levels()) return texel(Levels() - 1, 0, 0);
	// Convert EWA coordinates to appropriate scale for level
	st[0] = st[0] * pyramid[level]->uSize() - 0.5f;
	st[1] = st[1] * pyramid[level]->vSize() - 0.5f;
	dst0[0] *= pyramid[level]->uSize();
	dst0[1] *= pyramid[level]->vSize();
	dst1[0] *= pyramid[level]->uSize();
	dst1[1] *= pyramid[level]->vSize();

	// Compute ellipse coefficients to bound EWA filter region
	float A = dst0[1] * dst0[1] + dst1[1] * dst1[1] + 1;
	float B = -2 * (dst0[0] * dst0[1] + dst1[0] * dst1[1]);
	float C = dst0[0] * dst0[0] + dst1[0] * dst1[0] + 1;
	float invF = 1 / (A * C - B * B * 0.25f);
	A *= invF;
	B *= invF;
	C *= invF;

	// Compute the ellipse's $(s,t)$ bounding box in texture space
	float det = -B * B + 4 * A * C;
	float invDet = 1 / det;
	float uSqrt = std::sqrt(det * C), vSqrt = std::sqrt(A * det);
	int s0 = std::ceil(st[0] - 2 * invDet * uSqrt);
	int s1 = std::floor(st[0] + 2 * invDet * uSqrt);
	int t0 = std::ceil(st[1] - 2 * invDet * vSqrt);
	int t1 = std::floor(st[1] + 2 * invDet * vSqrt);

	// Scan over ellipse bound and compute quadratic equation
	T sum(0.f);
	float sumWts = 0;
	for (int it = t0; it <= t1; ++it) {
		float tt = it - st[1];
		for (int is = s0; is <= s1; ++is) {
			float ss = is - st[0];
			// Compute squared radius and filter texel if inside ellipse
			float r2 = A * ss * ss + B * ss * tt + C * tt * tt;
			if (r2 < 1) {
				int index =
					std::min((int)(r2 * WeightLUTSize), WeightLUTSize - 1);
				float weight = weightLut[index];
				sum += texel(level, is, it) * weight;
				sumWts += weight;
			}
		}
	}
	return sum / sumWts;
}

LUMINA_NAMESPACE_END