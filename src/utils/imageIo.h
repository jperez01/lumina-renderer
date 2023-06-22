#pragma once


#include "core/common.h"
#include "core/color.h"

LUMINA_NAMESPACE_BEGIN

std::unique_ptr<Color3f[]> readImage(const std::string& filename, Point2i& resolution);

LUMINA_NAMESPACE_END
