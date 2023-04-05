#include "SBREP/IO/SBrep.hpp"
#include <CGAL/draw_surface_mesh.h>
//#include <CGAL/draw_polyhedron.h>
#include <CGAL/Timer.h>

#include <SBREP/Core.hpp>
#include <SBREP/IO/OBJ.hpp>
#include <SBREP/SBREP_builder.hpp>
#include <iostream>
#include <string>

#include <CGAL/Projection_traits_3.h>

using namespace SBREP;

const FT SBREP_builder::max_distance_to_plane = FT(1);
const FT SBREP_builder::max_accepted_angle = FT(0.00125);
const std::size_t SBREP_builder::min_region_size = 1;

int main(int argc, char *argv[]) {
#ifdef __DEBUG__
  std::cout << std::endl
            << "* region_growing_on_polygon_mesh example started" << std::endl
            << std::endl;
#endif // __DEBUG__

  const std::string filename =
      (argc > 1) ? argv[1] : CGAL::data_file_path("data/testmesh.obj");
  Polygon_mesh pm;
  Face_group_map fgm;
  size_type num_groups;
  // if (!IO::read_OBJ(filename, pm, fgm, num_groups))
  //   return EXIT_FAILURE;
  IO::read_OBJ(filename, pm, fgm, num_groups);
  auto sbrep = SBREP_builder::Convert(pm, fgm, num_groups);
  if(sbrep.vertices.empty()) {
    std::cout << "Could not create SBrep object!!!" << std::endl;
    return 1;
  }
  if (argc > 2) {
    std::string file_name = argv[2];
    std::string sbrep_file_path = "results/" + file_name+".sbrep";
    std::string obj_file_path = "results/" + file_name+".obj";
    IO::write_OBJ(obj_file_path, sbrep.pm, sbrep.groups);
    IO::write_SBrep(sbrep_file_path, sbrep);
  }
  //CGAL::draw(sbrep.pm);
  return 0;
};
