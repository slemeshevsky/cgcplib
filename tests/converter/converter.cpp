#include <cgcplib/SBREPBuilder.hpp>
#include <iostream>
#include <string>
#include <CGAL/draw_surface_mesh.h>

int main(int argc, char *argv[]) {
  std::cout << std::endl
            << "region_growing_on_polygon_mesh example started" << std::endl
            << std::endl;

  const std::string filename = (argc>1) ? argv[1] : CGAL::data_file_path("data/testCube.obj");
  Polygon_mesh pm = SBREPBuilder::ReadObjFile();
  CGAL::draw(pm);
  return 0;
};
