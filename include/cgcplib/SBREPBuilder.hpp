#ifndef SBREPBUILDER_H
#define SBREPBUILDER_H

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Iterator_range.h>
#include <CGAL/Shape_detection/Region_growing/Region_growing.h>
#include <CGAL/Shape_detection/Region_growing/Region_growing_on_polygon_mesh.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/memory.h>

using Kernel = CGAL::Exact_predicates_exact_constructions_kernel;
using FT = typename Kernel::FT;
using Point_3 = Kernel::Point_3;

using Polygon_mesh = CGAL::Surface_mesh<Point_3>;
using Face_range = typename Polygon_mesh::Face_range;
using Neighbor_query =
    CGAL::Shape_detection::Polygon_mesh::One_ring_neighbor_query<Polygon_mesh>;
using Region_type =
    CGAL::Shape_detection::Polygon_mesh::Least_squares_plane_fit_region<
        Kernel, Polygon_mesh>;
using Sorting =
    CGAL::Shape_detection::Polygon_mesh::Least_squares_plane_fit_sorting<
        Kernel, Polygon_mesh, Neighbor_query>;

using Region = std::vector<std::size_t>;
using Regions = std::vector<Region>;
using Vertex_to_point_map = typename Region_type::Vertex_to_point_map;
using Region_growing = CGAL::Shape_detection::Region_growing<
    Face_range, Neighbor_query, Region_type, typename Sorting::Seed_map>;

class SBREPObject;

class SBREPBuilder {
public:
  const static FT max_distance_to_plane;
  const static FT max_accepted_angle;
  const static std::size_t min_region_size;
  //  const static Face_range face_range;

  static Polygon_mesh ReadObjFile(std::string obj_file_path);
  static Regions Convert(Polygon_mesh polygon_mesh);
  static int SaveRegions(Polygon_mesh polygon_mesh, Regions regions, const std::string path);
};

#endif /* SBREPBUILDER_H */
