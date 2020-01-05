#include <vector>
#include <utility>
#include <string>
#include <iostream>
#include <random>
#include <algorithm>

#include "../libs/include/voro++/voro++.hh"

#include "image.h"

#include "interpolation.h"
#include "power_diagram.h"

void generate_mapping(Image image, voro::container &container, std::vector< std::vector<int> > &pix_to_site, std::vector< std::vector<std::pair<double, double> > > &pix_to_coord, std::vector< std::vector< std::pair<int, int> > > &site_to_pix, int N)
{
    std::cout << "Generate a mapping..." << std::endl;

    int width = image.width;
    int height = image.height;

    pix_to_site = std::vector< std::vector<int> > (width, std::vector<int>(height, -1));
    pix_to_coord = std::vector< std::vector<std::pair<double, double> > > (width, std::vector< std::pair<double,double> >(height));
    site_to_pix = std::vector< std::vector< std::pair<int, int> > > (N);

    for(int x = 0; x < image.width; x++) {
        for(int y = 0; y < image.height; y++) {
            double rx, ry, rz;
            int site_id;            
            if(container.find_voronoi_cell((double)x,double(y),0., rx, ry, rz, site_id)) {
                pix_to_site[x][y] = site_id;
                pix_to_coord[x][y] = std::make_pair(rx, ry); 
                site_to_pix[site_id].push_back(std::make_pair(x,y));
            } else {
                std::cout << " ERROR CASE in generate_mapping" << std::endl;
            }
        }
    }
}

void generate_image_from_container(Image image, voro::container &container, std::string name, int N)
{
    Image quantized = image;

    std::vector< std::vector<int> > pix_to_site; 
    std::vector< std::vector<std::pair<double, double> > > pix_to_coord; 
    std::vector< std::vector< std::pair<int, int> > > site_to_pix;  

    generate_mapping(image, container, pix_to_site, pix_to_coord, site_to_pix, N);

    std::vector< double > site_weight(N, 0);

    for(int i = 0; i < N; i++) {
        int site_size = site_to_pix[i].size();
        for(std::pair<int, int> p : site_to_pix[i]) {
            site_weight[i] += (image.data[p.first][p.second].gs())/(double)site_size;
        }
    }

    for(int x = 0; x < image.width; x++) {
        for(int y = 0; y < image.height; y++) {
            int site = pix_to_site[x][y];
            int x_site = floor(pix_to_coord[x][y].first);
            int y_site = floor(pix_to_coord[x][y].second);

            quantized.data[x][y] = Pixel(site_weight[site]);//Pixel(image.data[x_site][y_site].gs());
        }
    }

    quantized.save_to_file(name);
}


// receives a gray-scaled image, and performs rejection sampling on it,
// returns a cloud of N points
void sampling_from_measure(Image image, std::vector< std::pair<double, double> > &sample, int N)    
{
    std::cout << "Performing initial rejection sampling on target image..." << std::endl;
    int width = image.width;
    int height = image.height;

    sample = std::vector< std::pair<double, double> >();

    // setup randomness
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist_row(0, height-1); // distribution in range [1, 6]
    std::uniform_int_distribution<std::mt19937::result_type> dist_col(0, width-1);

    std::uniform_real_distribution<> dist_unif(0, 1);

    // compute normalization scalar
    double normalization = 0.;
    for(int x = 0; x < width; x++) {
        for(int y = 0; y < height; y++) {
            normalization += (1-image.data[x][y].gs());
        }
    }

    int sampled = 0;
    while(sampled < N) {
        int x = dist_col(rng); 
        int y = dist_row(rng);
        double u = dist_unif(rng);

        // the grayscale defines a distribution on the image
        // since 0 < rho(p) <= 1 we have rho(p) <= nb_pixel * density_unif_pixel(p)
        // so we need to check whether u < rho(p)

        if(u <= (1-image.data[x][y].gs())/normalization) {
            bool already_found = (std::find(sample.begin(), sample.end(), std::make_pair((double)x,(double)y)) != sample.end());
            if(!already_found) {
                // accept the pixel
                sample.push_back(std::make_pair((double)x,(double)y));
                sampled++;
            }
        }
    }
}

void lloyd_sampling(Image image, int N)
{
    int height = image.height;
    int width = image.width;

    std::vector< std::pair<double, double> > sample;

    sampling_from_measure(image, sample, N);
    
    std::cout << "Performs Lloyd iterations to properly quantize the target image..." << std::endl; 

    for(int iter = 0; iter < 3; iter++) {
        int id;
        double x,y,z,r;         
        Image evolution = image;

        for(int i = 0; i < N; i ++) {
            char* name = new char[100];

            int x_id = floor(sample[i].first);
            int y_id = floor(sample[i].second);
            evolution.data[x_id][y_id] = Pixel(1.);

            sprintf(name, "debug_imgs/lloyd_iter_%d.png", iter); 

            evolution.save_to_file(name);

            delete[] name;
        }
       
        voro::container container(0, width, 0, height, -0.5, 0.5, 1, 1, 1, false, false, false, N);
        
        for(int i = 0; i < N; i++) {
            container.put(i, sample[i].first, sample[i].second, 0.);
        }

        // iterate over all cells
        voro::c_loop_all cla(container);
        voro::voronoicell c;

        if(cla.start()) do if (container.compute_cell(c,cla)) {
            cla.pos(id,x,y,z,r);
            double centr_x, centr_y, centr_z;
            c.centroid(centr_x, centr_y, centr_z);
            sample[id] = std::make_pair(x + 0.5*centr_x, y+0.5*centr_y);
        } while (cla.inc());


        char* name = new char[100];

        sprintf(name, "debug_imgs/lloyd_mapped_iter_%d.png", iter); 
           
        generate_image_from_container(image, container, name, N);
            
        delete[] name;
    }
}

void interpolation(std::string source_image, std::string target_image, int N)
{
    Image source = Image();
    source.load_from_file(source_image);
    source.convert_to_grayscale();

    Image target = Image();
    target.load_from_file(target_image);
    target.convert_to_grayscale();

    std::vector< std::pair<double, double> > sites {
        std::make_pair(1.,1.),
        std::make_pair(4.,19.),
        std::make_pair(50.,20.),
        std::make_pair(100.,35.),
        std::make_pair(16.,13.),
    };

    std::vector< double > weights { 1., 4., 2., 8., 1.};

    PowerDiagram pd = PowerDiagram(sites, weights, 128., 128.);
    pd.get_projection();

    lloyd_sampling(target, 500);
    target.save_to_file("sampled.png");
    
    return;
}
