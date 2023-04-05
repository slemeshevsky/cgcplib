#ifndef CORE_H
#define CORE_H
#include <SBREP/config.hpp>

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/HalfedgeDS_vector.h>
#include <CGAL/Iterator_range.h>
#include <CGAL/Plane_3.h>
#include <CGAL/memory.h>

#include <CGAL/Surface_mesh.h>

#include <CGAL/Shape_detection/Region_growing/Region_growing.h>
#include <CGAL/Shape_detection/Region_growing/Region_growing_on_polygon_mesh.h>

namespace SBREP {

typedef CGAL::Exact_predicates_exact_constructions_kernel Kernel;
typedef typename Kernel::FT FT;
typedef Kernel::Point_3 Point_3;
typedef Kernel::Plane_3 Plane_3;

using Polygon_mesh = CGAL::Surface_mesh<Point_3>;
using Face_range = typename Polygon_mesh::Face_range;
using size_type = Polygon_mesh::size_type;

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

typedef Polygon_mesh::Vertex_index vertex_descriptor;
typedef Polygon_mesh::Face_index face_descriptor;
typedef Polygon_mesh::Halfedge_index halfedge_descriptor;
typedef Polygon_mesh::Edge_index edge_descriptor;
typedef std::map<face_descriptor, size_type> Face_group_map;

typedef std::vector<halfedge_descriptor>
    Halfedges; ///< Вектор индексов полуребер
typedef std::map<vertex_descriptor, Halfedges>
    Vertex_halfedges_map; ///< Отображение (словарь) индекс вершины --- вектор
                          ///< полуребер
} // namespace SBREP
#endif /* CORE_H */
