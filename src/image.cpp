#include <vector>
#include <string>
#include <iostream>
#include <cmath>

#include "stb_image.h"
#include "stb_image_write.h"

#include "pixel.h"
#include "image.h"

#define linearize_rgb(x) (x <= 0.04045 ? x / 12.92 : pow((x+0.055)/1.055, 2.4))
#define gamma_compress_gs(x) (x <= 0.0031308 ? 12.92*x : 1.055*pow(x, 1/2.4)-0.055) 
#define clamp(x) std::max(std::min(x, 255), 0)

Image::Image()
{
    height = 0;
    width = 0;
    color = 0;
}

Image::Image(int h, int w, int c)
{
    height = h;
    width = w;
    color = c;

    data = std::vector<std::vector<Pixel>>(h, std::vector<Pixel>(w, Pixel(c)));
}

Image::Image(std::vector< std::vector<Pixel> > d, int h, int w, int c)
{
    data = d;
    height = h;
    width = w;
    color = c;
}

int Image::load_from_file(std::string file_name)
{
    int w, h, c;
    unsigned char* image_data = NULL;
    image_data = stbi_load(file_name.c_str(), &w, &h, &c, STBI_rgb);

    if(image_data == NULL) {
        std::cerr << "Error loading " << file_name << std::endl;
        width = 0;
        height = 0;
        color = 0;
        return 1;
    }

    std::vector<std::vector<Pixel>> d = std::vector<std::vector<Pixel>>(h, std::vector<Pixel>(w, Pixel(c)));

    for(int i = 0; i < h; i++) {
        for(int j = 0; j < w; j++) {
            int id = 3*(i*w + j);
            double r = (double)image_data[id]/255;
            double g = (double)image_data[id+1]/255;
            double b = (double)image_data[id+2]/255;
            d[i][j] = Pixel({r,g,b});

        }
    }

    height = h;
    width = w;
    data = d;
    color = 3;

    stbi_image_free(image_data);
    return 0;
}

void Image::convert_to_grayscale()
{
    if(color != 3) {
        std::cerr << "Cannot convert to grayscale from other than RGB\n";
        return;
    }

    std::vector< std::vector<Pixel> > new_data = std::vector<std::vector<Pixel>>(height, std::vector<Pixel>(width, Pixel(0.0)));

    for(int i = 0; i < height; i++) {
        for(int j = 0; j < width; j++) {

            double r = linearize_rgb(data[i][j].r());
            double g = linearize_rgb(data[i][j].g());
            double b = linearize_rgb(data[i][j].b());

            new_data[i][j] = Pixel(gamma_compress_gs(0.2126*r + 0.7152*g + 0.0722*b));
        }
    }

    data = new_data;
    color = 1;
}

int Image::save_to_file(std::string file_name)
{
    unsigned char* image = new unsigned char[height*width*3];
    for(int i = 0; i < height; i++) {
        for(int j = 0; j < width; j++) {
            int id = 3*(i*width + j);
            
            if(color == 3) {
                double r_d = data[i][j].r();
                double g_d = data[i][j].g();
                double b_d = data[i][j].b();

                int r = clamp((int)(255. * r_d));
                int g = clamp((int)(255. * g_d));
                int b = clamp((int)(255. * b_d));

                image[id] = r;
                image[id+1] = g;
                image[id+2] = b;
            } else if(color == 1) {
                double gs_d = data[i][j].gs();

                int gs = clamp((int)(255. * gs_d));
                
                image[id] = gs;
                image[id+1] = gs;
                image[id+2] = gs;
            }
        }
    }

    stbi_write_png(file_name.c_str(), width, height, STBI_rgb, image, 0);
    delete[] image;

    return 0;
}
