#pragma once
#include <string>
#include <vector>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>

#include <CGAL/memory.h>
#include <CGAL/Iterator_range.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Shape_detection/Region_growing/Region_growing.h>
#include <CGAL/Shape_detection/Region_growing/Region_growing_on_polygon_mesh.h>

using Kernel = CGAL::Exact_predicates_exact_constructions_kernel;
using FT = Kernel::FT;
using Point_3 = Kernel::Point_3;

using Polygon_mesh = CGAL::Surface_mesh<Point_3>;
using Face_range   = typename Polygon_mesh::Face_range;
using Neighbor_query = CGAL::Shape_detection::Polygon_mesh::One_ring_neighbor_query<Polygon_mesh>;
using Region_type    = CGAL::Shape_detection::Polygon_mesh::Least_squares_plane_fit_region<Kernel, Polygon_mesh>;
using Sorting        = CGAL::Shape_detection::Polygon_mesh::Least_squares_plane_fit_sorting<Kernel, Polygon_mesh, Neighbor_query>;

using Region  = std::vector<std::size_t>;
using Regions = std::vector<Region>;
using Vertex_to_point_map = typename Region_type::Vertex_to_point_map;
using Region_growing = CGAL::Shape_detection::Region_growing<Face_range, Neighbor_query, Region_type, typename Sorting::Seed_map>;

namespace cgcp::SBREP
{
    class SBREPBuilder
    {
    private:
        /* data */
        const FT max_distance_to_plane = FT(1);
        const FT max_accepted_angle = FT(45);
        const std::size_t min_region_size = 5;

    public:
        static Polygon_mesh ReadObjFile(std::string obj_file_path);
        static SBREPBuilder Convert(Polygon_mesh polygon_mesh);
    };
};