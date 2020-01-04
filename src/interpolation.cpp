#include <string>

#include "image.h"

#include "interpolation.h"



void interpolation(std::string source_image, std::string target_image, int N)
{
    Image source = Image();
    source.load_from_file(source_image);
    source.convert_to_grayscale();
    source.save_to_file("test.png");

    return;
}
