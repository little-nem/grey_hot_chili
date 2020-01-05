#include <vector>
#include <utility>
#include <string>
#include <iostream>
#include <random>
#include <algorithm>
#include <fstream>

#include "../libs/include/voro++/voro++.hh"

#include "image.h"

#include "interpolation.h"
#include "power_diagram.h"

#define DEBUG 0

void generate_mapping(Image image, voro::container &container, std::vector< std::vector<int> > &pix_to_site, std::vector< std::vector<std::pair<double, double> > > &pix_to_coord, std::vector< std::vector< std::pair<int, int> > > &site_to_pix, std::vector<double> &site_weight, int N)
{
    std::cout << "Generate a mapping..." << std::endl;

    int width = image.width;
    int height = image.height;

    pix_to_site = std::vector< std::vector<int> > (width, std::vector<int>(height, -1));
    pix_to_coord = std::vector< std::vector<std::pair<double, double> > > (width, std::vector< std::pair<double,double> >(height));
    site_to_pix = std::vector< std::vector< std::pair<int, int> > > (N);
    site_weight = std::vector< double >(N, 0);

    for(int x = 0; x < image.width; x++) {
        for(int y = 0; y < image.height; y++) {
            double rx, ry, rz;
            int site_id;            
            if(container.find_voronoi_cell((double)x,double(y),0., rx, ry, rz, site_id)) {
                pix_to_site[x][y] = site_id;
                pix_to_coord[x][y] = std::make_pair(rx, ry); 
                site_to_pix[site_id].push_back(std::make_pair(x,y));
                site_weight[site_id] += image.data[x][y].gs(); 
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
    std::vector< double > site_weight;
    generate_mapping(image, container, pix_to_site, pix_to_coord, site_to_pix, site_weight, N);



    for(int x = 0; x < image.width; x++) {
        for(int y = 0; y < image.height; y++) {
            int site = pix_to_site[x][y];
            quantized.data[x][y] = Pixel(site_weight[site]/site_to_pix[site].size());//Pixel(image.data[x_site][y_site].gs());
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

void lloyd_sampling(Image image, std::vector< std::pair<double, double> >&sample, std::vector< double > &masses, int N)
{
    int height = image.height;
    int width = image.width;

    int max_iter = 3;

    sampling_from_measure(image, sample, N);
    
    std::cout << "Performs Lloyd iterations to properly quantize the target image..." << std::endl; 

    for(int iter = 0; iter < max_iter; iter++) {
        int id;
        double x,y,z,r;         
        Image evolution = image;

        if(DEBUG) {
            for(int i = 0; i < N; i ++) {
                char* name = new char[100];

                int x_id = floor(sample[i].first);
                int y_id = floor(sample[i].second);
                evolution.data[x_id][y_id] = Pixel(1.);

                sprintf(name, "debug_imgs/lloyd_iter_%d.png", iter); 

                evolution.save_to_file(name);

                delete[] name;
            }
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


        if(DEBUG) {
            char* name = new char[100];

            sprintf(name, "debug_imgs/lloyd_mapped_iter_%d.png", iter); 
           
            generate_image_from_container(image, container, name, N);
            
            delete[] name;
        }

        if(iter+1 == max_iter) {
            std::vector< std::vector<int> > pix_to_site; 
            std::vector< std::vector<std::pair<double, double> > > pix_to_coord; 
            std::vector< std::vector< std::pair<int, int> > > site_to_pix;  
            std::vector< double > site_weight;
            generate_mapping(image, container, pix_to_site, pix_to_coord, site_to_pix, masses, N);
        }
    }



}

double compute_total_mass(Image image)
{
    double mass = 0.;
    for(int i = 0; i < image.width; i++) {
        for(int j = 0; j < image.height; j++) {
            mass += image.data[i][j].gs();
        }
    }
    return mass;
}

void interpolation(std::string source_image, std::string target_image, int N)
{
    N = 500;
    double step = 10.;
    Image source = Image();
    source.load_from_file(source_image);
    source.convert_to_grayscale();

    double source_total_mass = compute_total_mass(source);

    Image target = Image();
    target.load_from_file(target_image);
    target.convert_to_grayscale();

    double target_total_mass = compute_total_mass(target);

    std::vector< std::pair<double, double> > sites {
        std::make_pair(1.,1.),
        std::make_pair(4.,19.),
        std::make_pair(50.,20.),
        std::make_pair(100.,35.),
        std::make_pair(16.,13.),
    };

    /*std::vector< double > weights { 1., 4., 2., 8., 1.};

    PowerDiagram pd = PowerDiagram(sites, weights, 128., 128.);
    pd.get_projection();*/

    std::vector< std::pair<double, double> >target_sample;
    std::vector< double > target_masses;

    lloyd_sampling(target, target_sample, target_masses, N);

    std::vector< double > weights(N, 10.);

    std::ofstream outputFile("mse_step10_N500_10Kiter.txt");

    for(int gradient_iter = 0; gradient_iter < 10000; gradient_iter++) {
        PowerDiagram pd = PowerDiagram(target_sample, weights, (double)target.width, (double)target.height);

        std::vector< std::vector<int> > pix_to_site; 
        std::vector< std::vector<std::pair<double, double> > > pix_to_coord; 
        std::vector< std::vector< std::pair<int, int> > > site_to_pix;  
        std::vector< double > site_weight;
        generate_mapping(source, *pd.container, pix_to_site, pix_to_coord, site_to_pix, site_weight, N);

        if(DEBUG) {
            std::cout << "masses stuff" << target_masses[0] << " " << site_weight[0] << std::endl;
            std::cout << "masses stuff" << target_masses[20] << " " << site_weight[20] << std::endl;
        }
        
        std::vector< double > gradient(N,0);
        double mse = 0;
        for(int p = 0; p < N; p++) {
            gradient[p] = target_masses[p]/target_total_mass - site_weight[p]/source_total_mass;
            mse += (gradient[p]*gradient[p])/N;
            weights[p] = std::max(1e-4, weights[p] + step * gradient[p]);
        }
        if(DEBUG) {
            std::cout << "grad : " << gradient[0] << " " << gradient[20] << std::endl;
            std::cout << "weight : " << weights[0] << " " << weights[20] << std::endl;
        }
        std::cout << gradient_iter << "; " << mse << std::endl;
        outputFile << gradient_iter << "; " << mse << std::endl;

        // perform update

    }

    return;
}
