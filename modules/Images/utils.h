#pragma once

#include <panda/graphics/Image.h>
#include <panda/types/ImageWrapper.h>
#include <string>
#include <FreeImage.h>

namespace panda 
{

std::string getFilterString(const std::vector<FREE_IMAGE_FORMAT>& formats);
std::string getSaveFilterString();
std::string getOpenFilterString();

graphics::Image loadImage(const std::string& fileName);
void saveImage(const graphics::Image& img, const std::string& fileName);

graphics::Image convertToImage(FIBITMAP* dib);
FIBITMAP* convertFromImage(const graphics::Image& img);

} // namespace Panda
