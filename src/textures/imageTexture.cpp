#include "imageTexture.h"
#include "utils/imageIo.h"

LUMINA_NAMESPACE_BEGIN

template<typename Tmemory, typename Treturn>
ImageTexture<Tmemory, Treturn>::ImageTexture(const PropertyList& list)
{
	std::string filename = list.getString("filename");
	bool doTrilinear = list.getBoolean("doTrilinear", true), gamma = list.getBoolean("gamma", true);
	float maxAniso = list.getFloat("maxAniso", 1.0f), scale = list.getFloat("scale", 1.0f);
	ImageWrap wrap = ImageWrap::Clamp;

	std::map<std::string, TextureType> types = {
		{"albedo", Albedo},
		{"normal", Normal},
		{"metallic", Metallic},
		{"roughness", Roughness},
		{"ao", AO}
	};
	std::string textureType = list.getString("type");
	type = types[textureType];

	mipmap = getTexture(filename, doTrilinear, maxAniso, wrap, scale, gamma);

	mapping = std::unique_ptr<TextureMapping2D>(new UVMapping2D());
}

template<typename Tmemory, typename Treturn>
ImageTexture<Tmemory, Treturn>::ImageTexture(std::unique_ptr<TextureMapping2D> m, 
	const std::string& filename, bool doTrilinear, float maxAniso, ImageWrap wrap, 
	float scale, bool gamma) : mapping(m) {
	mipmap = getTexture(filename, doTrilinear, maxAniso, wrapMode, scale, gamma);
}

template<typename Tmemory, typename Treturn>
Treturn ImageTexture<Tmemory, Treturn>::evaluate(Intersection& its) const
{
	Vector2f dstdx, dstdy;
	Point2f st = mapping->map(its, dstdx, dstdy);
	Tmemory mem = mipmap->lookup(st, dstdx, dstdy);

	Treturn returnValue;
	convertOut(mem, returnValue);

	return returnValue;
}

template<typename Tmemory, typename Treturn>
MipMap<Tmemory>* ImageTexture<Tmemory, Treturn>::getTexture(const std::string& filename, 
	bool doTrilinear, float maxAniso, ImageWrap wrap, float scale, bool gamma)
{
	TextureInfo texInfo(filename, doTrilinear, maxAniso, wrap, scale, gamma);
	if (textures.find(texInfo) != textures.end())
		return textures[texInfo].get();

	Point2i resolution;
	std::unique_ptr<Color3f[]> texels = readImage(filename, resolution);
	MipMap<Tmemory>* mipmap = nullptr;

	if (texels) {
		std::unique_ptr<Tmemory[]> convertedTexels(new Tmemory[resolution.x() * resolution.y()]);
		for (int i = 0; i < resolution.x() * resolution.y(); i++)
			convertIn(texels[i], convertedTexels[i], scale, gamma);

		mipmap = new MipMap<Tmemory>(resolution, convertedTexels.get(), doTrilinear, maxAniso, wrap);
	}
	else {
		Tmemory oneValue = scale;
		resolution = Point2i(1, 1);
		mipmap = new MipMap<Tmemory>(resolution, &oneValue);
	}
	textures[texInfo].reset(mipmap);

	return mipmap;
}

template<typename Tmemory, typename Treturn>
void ImageTexture<Tmemory, Treturn>::convertIn(const Color3f& from, Color3f& to, float scale, bool gamma)
{
	for (int i = 0; i < 3; i++) {
		to[i] = scale * (gamma ? inverseGammaCorrect(from[i]) : from[i]);
	}
}

template<typename Tmemory, typename Treturn>
void ImageTexture<Tmemory, Treturn>::convertIn(const Color3f& from, float& to, float scale, bool gamma)
{
	to = scale * (gamma ? inverseGammaCorrect(from.y()) : from.y());
}

template<typename Tmemory, typename Treturn>
void ImageTexture<Tmemory, Treturn>::convertOut(Color3f& from, Color3f& to)
{
	to = from;
}

template<typename Tmemory, typename Treturn>
void ImageTexture<Tmemory, Treturn>::convertOut(float from, float& to)
{
	to = from;
}

template <typename Tmemory, typename Treturn>
std::map<TextureInfo, std::unique_ptr<MipMap<Tmemory>>>
ImageTexture<Tmemory, Treturn>::textures;

#define COMMA ,

LUMINA_TEMPLATED_REGISTER_CLASS(ImageTexture<float COMMA float>, FloatTexture, "floatTexture")
LUMINA_TEMPLATED_REGISTER_CLASS(ImageTexture<Color3f COMMA Color3f>, ColorTexture, "colorTexture")

LUMINA_NAMESPACE_END