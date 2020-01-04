#ifndef pixel_h_INCLUDED
#define pixel_h_INCLUDED

class Pixel
{
    public:
        std::vector<double> values;

        Pixel(double value);
        Pixel(int color);
        Pixel(std::vector<double> v);
        
        double r();
        double g();
        double b();
        double gs();
 
        double access_value(int id);
};

#endif // pixel_h_INCLUDED
