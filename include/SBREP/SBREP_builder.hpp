#ifndef SBREPBUILDER_H
#define SBREPBUILDER_H

#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <SBREP/Core.hpp>
#include <SBREP/SBREP_object.hpp>

namespace SBREP {

class SBREP_builder {
public:
  const static FT max_distance_to_plane;
  const static FT max_accepted_angle;
  const static std::size_t min_region_size;

  static std::pair<Regions, Face_group_map>
  Group_triangles_by_plane(Polygon_mesh polygon_mesh);
  static std::tuple<Halfedges, Vertex_halfedges_map, vertex_descriptor>
  Initialize_ds(Polygon_mesh mesh, Region region);
  static std::vector<vertex_descriptor>
  Extract_linear_ring(Vertex_halfedges_map PtE, vertex_descriptor vd,
                      Halfedges &HE, Polygon_mesh &mesh);

  static std::vector<std::vector<vertex_descriptor>>
  Extract_holes(Vertex_halfedges_map PtE, Halfedges &HE, Polygon_mesh &mesh);

  static Regions Group_triangles(Polygon_mesh mesh, Regions planes,
                                 Face_group_map &fgm, size_type &num_groups);
  static Plane_3 Get_plane(Region region, Polygon_mesh &mesh);
  static int Get_group_id(Region region, Polygon_mesh &mesh,
                          Face_group_map &fgm);
  static SBrep Convert(Polygon_mesh polygon_mesh, Face_group_map fgm,
                       size_type num_groups);

  static std::set<Edge_idx> add_edges(std::vector<vertex_descriptor> verts,
                                      Polygon_mesh &pm,
                                      Face_group_map &face_group_map,
                                      Face_group_map &face_plane_map,
                                      SBrep &obj);

  // static std::vector<Polyline_idx> add_outer_polylines(std::set<Edge_idx>
  // eids,
  //                                                      SBrep &obj);
  static std::pair<Regions, std::vector<std::size_t>>
  Group_triangles_by_group(Region planar_group, Face_group_map &fgm);
  static std::map<std::pair<int, int>, std::vector<Edge_idx>>
  Group_edges_by_groups(std::set<Edge_idx> eids, Polygon_mesh &pm,
                        Face_group_map &face_group_map,
                        SBrep &obj);
  static std::map<int, std::vector<Edge_idx>>
  Group_edges_by_connection(std::vector<Edge_idx> edges, SBrep &obj);
  static std::vector<Polyline_idx> add_outer_polylines(
      std::map<std::pair<int, int>, std::vector<Edge_idx>> grouped_edges,
      SBrep &obj);
  static std::vector<Edge_idx>::iterator
  find_connected_component(std::vector<Edge_idx>::iterator first,
                           std::vector<Edge_idx>::iterator last, SBrep &obj);

private:
  static halfedge_descriptor
  select_edge(halfedge_descriptor he, Halfedges halfedges, Polygon_mesh &mesh);
};
} // namespace SBREP
#endif /* SBREPBUILDER_H */
