#pragma once

#include "mipmap.h"
#include <map>

LUMINA_NAMESPACE_BEGIN

struct TextureInfo {
	TextureInfo(const std::string& f, bool dt, float ma, ImageWrap wm, float sc, bool gamma)
		: filename(f), doTrilinear(dt), maxAniso(ma), wrapMode(wm), scale(sc), gamma(gamma) {}

	std::string filename;
	bool doTrilinear, gamma;
	float maxAniso, scale;
	ImageWrap wrapMode;

	bool operator<(const TextureInfo& info2) const {
		if (filename != info2.filename) return filename < info2.filename;
		if (doTrilinear != info2.doTrilinear) return doTrilinear < info2.doTrilinear;
		if (gamma != info2.gamma) return gamma < info2.gamma;
		if (maxAniso != info2.maxAniso) return maxAniso < info2.maxAniso;
		if (scale != info2.scale) return scale < info2.scale;
		if (gamma != info2.gamma) return gamma < info2.gamma;

		return wrapMode < info2.wrapMode;
	}
};

template <typename Tmemory, typename Treturn>
class ImageTexture : public Texture<Tmemory> {
public:
	ImageTexture(const PropertyList& list);
	ImageTexture(std::unique_ptr<TextureMapping2D> m, const std::string& filename,
		bool doTrilinear, float maxAniso, ImageWrap wrap, float scale, bool gamma);
	Treturn evaluate(Intersection& its) const;

	EClassType getTemplatedClassType() const { 
		if (type == Albedo)
			return EColorTexture;
		else if (type == Metallic || type == Roughness)
			return EFloatTexture;
		else if (type == Default)
			return ETexture;
	}

	static void clearCache() {
		textures.erase(textures.begin(), textures.end());
	}

private:
	static MipMap<Tmemory>* getTexture(const std::string& filename, bool doTrilinear,
		float maxAniso, ImageWrap wrap, float scale, bool gamma);
	static void convertIn(const Color3f& from, Color3f& to, float scale, bool gamma);
	static void convertIn(const Color3f& from, float& to, float scale, bool gamma);
	static void convertOut(Color3f& from, Color3f& to);
	static void convertOut(float from, float& to);

	std::unique_ptr<TextureMapping2D> mapping;
	MipMap<Tmemory>* mipmap;
	static std::map<TextureInfo, std::unique_ptr<MipMap<Tmemory>>> textures;
};

LUMINA_NAMESPACE_END
