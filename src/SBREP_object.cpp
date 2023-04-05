#include "SBREP/Core.hpp"
#include <SBREP/SBREP_object.hpp>
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <ostream>
#include <vector>

namespace SBREP {

SBrep::SBrep() {
  vertices = std::vector<Vertex_idx>();
  edges = std::vector<Edge_idx>();
  polylines = std::vector<Polyline_idx>();
  loops = std::vector<Loop_idx>();
  faces = std::vector<Face_idx>();

  vconn = std::map<Vertex_idx, Vertex_connectivity>();
  econn = std::map<Edge_idx, Edge_connectivity>();
  poly_conn = std::map<Polyline_idx, Polyline_connectivity>();
  loop_conn = std::map<Loop_idx, Loop_connectivity>();
  fconn = std::map<Face_idx, Face_connectivity>();
  points = std::map<Vertex_idx, Point_3>();
  planes = std::map<Face_idx, Plane_3>();
};

SBrep::~SBrep(){};

Vertex_idx SBrep::add_vertex(Point_3 p, vertex_descriptor vd) {
  //  auto max_vid = *std::max_element(points.begin(), points.end());
  auto vertex_in_map =
      std::find_if(std::begin(points), std::end(points),
                   [p](const auto &mo) { return mo.second == p; });
  if (vertex_in_map == points.end()) {
    // vertex_descriptor vd = pm.add_vertex(p);
    auto vid = static_cast<std::size_t>(vd);
    vertices.push_back(vid);
    points[vid] = p;
    return vid;
  } else
    return vertex_in_map->first;
}

Edge_idx SBrep::add_edge(Vertex_idx v0, Vertex_idx v1, int g0,
                         int g1, int plane0, int plane1) {
  auto verts = std::pair<Vertex_idx, Vertex_idx>(v0, v1);
  auto verts_opp = std::pair<Vertex_idx, Vertex_idx>(v1, v0);
  auto gr = std::pair<int, int>(g0, g1);
  auto pl = std::pair<int, int>(plane0, plane1);
  auto edge_in_map =
      std::find_if(std::begin(econn), std::end(econn), [verts](const auto &mo) {
        return mo.second.vertices_ == verts;
      });
  auto edge_opp_in_map = std::find_if(
      std::begin(econn), std::end(econn),
      [verts_opp](const auto &mo) { return mo.second.vertices_ == verts_opp; });

  if ((edge_in_map == std::end(econn)) &&
      (edge_opp_in_map == std::end(econn))) {
    auto eid = edges.size();
    edges.push_back(eid);
    Edge_connectivity e;
    e.vertices_ = verts;
    e.groups_ = gr;
    e.planes_ = pl;
    econn[eid] = e;
    vconn[v0].edges_.push_back(eid);
    vconn[v1].edges_.push_back(eid);
    return eid;
  } else if (edge_in_map != std::end(econn))
    return edge_in_map->first;
  else
    return edge_opp_in_map->first;
}

Polyline_idx SBrep::add_polyline(std::vector<Edge_idx> polyline) {
  std::sort(polyline.begin(), polyline.end());
  auto poly_in_map = std::find_if(
      std::begin(poly_conn), std::end(poly_conn),
      [polyline](const auto &mo) { return mo.second.edges_ == polyline; });
  if (poly_in_map == std::end(poly_conn)) {
    auto pid = polylines.size();
    polylines.push_back(pid);
    poly_conn[pid].edges_ = polyline;
    for (auto eid : polyline)
      econn[eid].polyline_ = pid;
    return pid;
  } else
    return poly_in_map->first;
}

Loop_idx SBrep::add_loop(std::vector<Polyline_idx> loop) {
  std::sort(loop.begin(), loop.end());

#ifdef __DEBUG__

#endif // __DEBUG__

  auto loop_in_map = std::find_if(
      std::begin(loop_conn), std::end(loop_conn),
      [loop](const auto &mo) { return mo.second.polylines_ == loop; });
  if (loop_in_map == std::end(loop_conn)) {
    auto lid = loops.size();
    loops.push_back(lid);
    loop_conn[lid].polylines_ = loop;
    for (auto pid : loop) {
      poly_conn[pid].loops_.push_back(lid);
    }
    return lid;
  } else
    return loop_in_map->first;
}

Face_idx SBrep::add_face(Loop_idx outer_loop, std::vector<Loop_idx> hole_loops,
                         int group_id) {
  auto face_in_map = std::find_if(std::begin(fconn), std::end(fconn),
                                  [outer_loop](const auto &mo) {
                                    return mo.second.outer_loop_ == outer_loop;
                                  });
  if (face_in_map == std::end(fconn)) {
    auto fid = faces.size();
    faces.push_back(fid);
    fconn[fid].outer_loop_ = outer_loop;
    fconn[fid].hole_loops_ = hole_loops;
    fconn[fid].group_id_ = group_id;
    return fid;
  } else
    return face_in_map->first;
}

void SBrep::add_pm_face(
    std::vector<vertex_descriptor> outer_verts,
    std::vector<std::vector<vertex_descriptor>> holes_verts) {
  std::vector<vertex_descriptor> vertices;
  for (auto v = outer_verts.begin(); v != outer_verts.end() - 1; v++) {
    vertices.push_back(*v);
  }

#ifdef __DEBUG_
  std::cout << "Vertices: " << std::endl;
  for (auto &vd : vertices) {
    std::cout << "\t " << vd << std::endl;
  }
#endif // __DEBUG__

  for (auto &hole : holes_verts) {
    //
    vertices.push_back(outer_verts.front());
    for (auto v = hole.begin(); v != hole.end() - 1; v++) {
      vertices.push_back(*v);
    }
  }

  auto fd = pm.add_face(vertices);
  if (fd == Polygon_mesh::null_face()) {
    std::cerr << "The face could not be added." << std::endl;
    assert(fd != Polygon_mesh::null_face());
  }
}

  std::pair<Edge_idx, Edge_idx> SBrep::get_adjacent_edges(Edge_idx edge) {
    auto &[v0, v1] = econn[edge].vertices_;
    auto ev0 = vconn[v0].edges_;
    Edge_idx e0 = ev0[0] != edge ? ev0[0] : ev0[1];
    auto ev1 = vconn[v1].edges_;
    Edge_idx e1 = ev1[0] != edge ? ev1[0] : ev1[1];
    return std::pair<Edge_idx, Edge_idx>(e0, e1);
  }
  
void SBrep::print_loop(Loop_idx id) {
  auto loop_exists = std::find(loops.begin(), loops.end(), id);
  if (loop_exists == loops.end()) {
    std::cout << "****** Loop with id " << id << "does not exist." << std::endl;
    assert(loop_exists != loops.end());
  }
  std::cout << "****************************************" << std::endl;

  std::cout << "Loop " << id << " contains: " << loop_conn[id].polylines_.size()
            << " polylines" << std::endl;
  for (auto pid : loop_conn[id].polylines_) {
    std::cout << "Polyline: " << pid << " contains "
              << poly_conn[pid].edges_.size() << " edges" << std::endl;
    std::cout << "Vertices: " << std::endl;
    for(auto eid : poly_conn[pid].edges_) {
      auto vid = econn[eid].vertices_.first;
      auto p = points[vid];
      std::cout << vid << " (" << p.x() << ", " << p.y() << ", " << p.z()
                << "); ";
    }
    std::cout << std::endl;
  }
  std::cout << "****************************************" << std::endl;
}
} // namespace SBREP
