#ifndef power_diagram_h_INCLUDED
#define power_diagram_h_INCLUDED

#include "../libs/include/voro++/voro++.hh"

class PowerDiagram
{
    public:
        int nb_sites;
        float lifting_constant;
        std::vector< std::pair<double, double> > sites;
        std::vector< double > weights; 
        voro::container *container = NULL;
        std::vector< std::vector< std::pair<double, double> > > sites_edges;

        PowerDiagram();
        PowerDiagram(std::vector< std::pair<double, double> > s, std::vector< double > w, double x_range, double y_range);

        void get_projection();

        ~PowerDiagram();
};

#endif // power_diagram_h_INCLUDED
