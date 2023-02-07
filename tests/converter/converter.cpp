#include <cgcplib/SBREPBuilder.hpp>
#include <iostream>
#include <string>
#include <CGAL/draw_surface_mesh.h>

int main(int argc, char *argv[]) {
  std::cout << std::endl
            << "region_growing_on_polygon_mesh example started" << std::endl
            << std::endl;

  Polygon_mesh pm = SBREPBuilder::ReadObjFile("/home/svl/Projects/Cxx/cgcplib/data/furniture_xena.obj");
  CGAL::draw(pm);
  return 0;
};
