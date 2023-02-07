#include <CGAL/draw_surface_mesh.h>
#include <cgcplib/SBREPBuilder.hpp>
#include <iostream>
#include <string>

const FT SBREPBuilder::max_distance_to_plane = FT(1);
const FT SBREPBuilder::max_accepted_angle = FT(1);
const std::size_t SBREPBuilder::min_region_size = 1;

int main(int argc, char *argv[]) {
  std::cout << std::endl
            << "region_growing_on_polygon_mesh example started" << std::endl
            << std::endl;

  const std::string filename =
      (argc > 1) ? argv[1] : CGAL::data_file_path("data/tube.obj");
  Polygon_mesh pm = SBREPBuilder::ReadObjFile(filename);
  Regions regions = SBREPBuilder::Convert(pm);
  if (argc > 2) {
    const std::string filename = argv[2];
    if (argc < 4) {
      std::string fullpath = "/results/" + filename;
      SBREPBuilder::SaveRegions(pm, regions, fullpath, "off");
    } else {
      std::string extension = argv[3];
      std::string fullpath = "./results/" + filename;
      SBREPBuilder::SaveRegions(pm, regions, fullpath, extension);
    }
  };
  // CGAL::draw(pm);
  return 0;
};
