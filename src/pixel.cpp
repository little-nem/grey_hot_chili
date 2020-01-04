#include <vector>

#include "pixel.h"

Pixel::Pixel(double value)
{
    values = std::vector<double>{value};
}

Pixel::Pixel(int color)
{
    values = std::vector<double>(color, 0.);
}

Pixel::Pixel(std::vector<double> v)
{
    values = v;
}

double Pixel::r()
{
    return access_value(0);
}

double Pixel::g()
{
    return access_value(1);
}

double Pixel::b()
{
    return access_value(2);
}

double Pixel::gs()
{
    return access_value(0);
}

double Pixel::access_value(int id)
{
    if(id < (int)values.size()) return values[id];
    return 0.;
}
