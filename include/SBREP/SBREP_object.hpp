#ifndef SBREPOBJECT_H
#define SBREPOBJECT_H

#include <SBREP/Core.hpp>
#include <cstddef>
#include <map>
#include <vector>

namespace SBREP {

typedef std::size_t Vertex_idx;
typedef std::size_t Edge_idx;
typedef std::size_t Polyline_idx;
typedef std::size_t Loop_idx;
typedef std::size_t Face_idx;

struct Vertex_connectivity {
  std::vector<Edge_idx> edges_;
};

struct Edge_connectivity {
  std::pair<Vertex_idx, Vertex_idx> vertices_;
  Polyline_idx polyline_;
  std::pair<Face_idx, Face_idx> faces_;
  std::pair<int, int> planes_;
  std::pair<int, int> groups_;
};

struct Polyline_connectivity {
  std::vector<Edge_idx> edges_;
  std::vector<Loop_idx> loops_;
  std::vector<Face_idx> faces_;
};

struct Loop_connectivity {
  std::vector<Polyline_idx> polylines_;
  std::vector<Face_idx> faces_;
};

struct Face_connectivity {
  int group_id_;
  Loop_idx outer_loop_;
  std::vector<Loop_idx> hole_loops_;
};

class SBrep {
public:
  SBrep();
  Vertex_idx add_vertex(Point_3 p, vertex_descriptor vd);
  Edge_idx add_edge(Vertex_idx v0, Vertex_idx v1, int g0,
                    int g1, int plane0, int plane1);
  Polyline_idx add_polyline(std::vector<Edge_idx> polyline);
  Loop_idx add_loop(std::vector<Polyline_idx> loop);
  Face_idx add_face(Loop_idx outer_loop, std::vector<Loop_idx> hole_loops,
                    int group_id = 0);
  
  void add_pm_face(std::vector<vertex_descriptor> outer_verts,
                   std::vector<std::vector<vertex_descriptor>> holes_verts);
  std::pair<Edge_idx, Edge_idx> get_adjacent_edges(Edge_idx e);
  void print_loop(Loop_idx id);
  
  // void construct_polylines();
  // void construct_loops();

  // bool is_polyline_closed(std::vector<Edge_idx> eds);
  // bool is_path_closed(std::vector<Polyline_idx> polys);
  virtual ~SBrep();

  std::vector<Vertex_idx> vertices;
  std::vector<Edge_idx> edges;
  std::vector<Polyline_idx> polylines;
  std::vector<Loop_idx> loops;
  std::vector<Face_idx> faces;
  // std::vector<Plane_3> planes;

  std::map<Vertex_idx, Vertex_connectivity> vconn;
  std::map<Edge_idx, Edge_connectivity> econn;
  std::map<Polyline_idx, Polyline_connectivity> poly_conn;
  std::map<Loop_idx, Loop_connectivity> loop_conn;
  std::map<Face_idx, Face_connectivity> fconn;
  std::map<Vertex_idx, Point_3> points;
  std::map<Face_idx, Plane_3> planes;
  Regions groups;

  // private:
  Polygon_mesh pm;
};

} // namespace SBREP

#endif /* SBREPOBJECT_H */
