#ifndef image_h_INCLUDED
#define image_h_INCLUDED

#include <vector>

#include "pixel.h"

class Image
{
    public:
        std::vector< std::vector<Pixel>> data;
        int height;
        int width;
        int color;

        Image();
        Image(int h, int w, int color);
        Image(std::vector< std::vector<Pixel> > data, int h, int w, int color);

        void convert_to_grayscale();
        
        int load_from_file(std::string file_name);
        int save_to_file(std::string file_name);
};

#endif // image_h_INCLUDED

