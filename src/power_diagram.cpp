#include <vector>
#include <utility>
#include <algorithm>
#include <iostream>
#include <cmath>

#include "../libs/include/voro++/voro++.hh"

#include "power_diagram.h"

PowerDiagram::PowerDiagram()
{
    nb_sites = 0;
    lifting_constant = 0;
    container = new voro::container(0.,1.,0.,1.,0.,1., 1, 1, 1, false, false, false, 1);
}

PowerDiagram::PowerDiagram(std::vector< std::pair<double, double> > s, std::vector< double > w, double x_range, double y_range)
{
    nb_sites = s.size();
    sites = s;
    weights = w;

    // to ensure that we will not compute square roots of non-positive numbers
    lifting_constant = 4 * (*max_element(weights.begin(),weights.end()));

    // create the container
    container = new voro::container (0., x_range, 0., y_range, 0., sqrt(lifting_constant), 1, 1, 1, false,false,false, nb_sites);

    // we will add the lifted points to a container
    // remember that the lifting is (x, y) -> (x, y, sqrt(c - w)) 
    for(int i = 0; i < nb_sites; i++) {
        container->put(i, s[i].first, s[i].second, sqrt(lifting_constant - w[i]));
    }
    container->draw_cells_gnuplot("cells.gnu");
}

void PowerDiagram::get_projection()
{
    sites_edges = std::vector< std::vector < std::pair<double, double> >  >(nb_sites, std::vector< std::pair<double, double> >());

    if(container != NULL) {
        voro::c_loop_all cla(*container);
        voro::voronoicell c;
        int i;
        double x,y,z,r;
        if(cla.start()) do if (container->compute_cell(c,cla)) {
            int relevant_vertex = c.p;
            // Get the position and ID information for the particle
            // currently being considered by the loop. Ignore the radius
            // information.
            cla.pos(i,x,y,z,r);

            for(int j = 0; j < relevant_vertex; j++) {
                double v_x = x + 0.5*c.pts[3*j];
                double v_y = y + 0.5*c.pts[3*j+1];
                double v_z = z + 0.5*c.pts[3*j+2];

                if(abs(v_z) <= 1e-10) {
                    // consider that v_z = 0
                    sites_edges[i].push_back(std::make_pair(v_x, v_y));
                }
            }
        } while (cla.inc());
    }
}

PowerDiagram::~PowerDiagram()
{
    delete container;
}
