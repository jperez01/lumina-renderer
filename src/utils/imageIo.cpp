#include "imageIo.h"
#include "primitives/vector.h"

#include <stb_image.h>

LUMINA_NAMESPACE_BEGIN 

std::unique_ptr<Color3f[]> readImage(const std::string& filename, Point2i& resolution)
{
	std::filesystem::path resolvedName =
		getFileResolver()->resolve(filename);
	int width, height, channels;
	unsigned char* img = stbi_load(resolvedName.string().c_str(), &width, &height, &channels, 0);

	std::unique_ptr<Color3f[]> result;
	std::vector<Color3f> vectorResult(width * height);
	if (img) {
		resolution = Point2i(width, height);
		result = std::make_unique<Color3f[]>(width * height);

		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				int index = (i * width + j);
				int convertedIndex = (i * width + j) * 3;

				result[index] = Color3f(img[convertedIndex] / 255.0f,
					img[convertedIndex +1] / 255.0f,
					img[convertedIndex +2] / 255.0f);

				vectorResult[index] = result[index];
			}
		}
	}
	else {
		throw LuminaException("Image could not be loaded at path: %s", filename);
	}

	return result;
}

LUMINA_NAMESPACE_END
