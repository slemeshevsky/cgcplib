#ifndef SBREP_H
#define SBREP_H

#include <CGAL/number_utils.h>
#include <math.h>
#include <SBREP/SBREP_object.hpp>

namespace SBREP {
namespace IO {
void write_SBrep(const std::string file_path, SBrep obj) {
  std::ofstream os(file_path);
  os << obj.points.size() << std::endl;
  for (auto v : obj.points) {
    auto p = v.second;
    os << v.first << " ";
    os << p.x() << " " << p.y() << " " << p.z() << std::endl;
  }

  os << obj.edges.size() << std::endl;
  for (auto eid : obj.edges) {
     auto e = obj.econn[eid];
     os << eid << " ";
     os << e.vertices_.first << " " << e.vertices_.second << std::endl;
  }

  os << obj.polylines.size() << std::endl;
  for (auto pid : obj.polylines) {
    auto poly = obj.poly_conn[pid];
    os << pid << " ";
    os << poly.edges_.size() << " ";    
    for (auto eid : poly.edges_) {
      os << eid << " ";
    }
    os << std::endl;
  }

  os << obj.loops.size() << std::endl;
  for(auto lid : obj.loops) {
    auto loop = obj.loop_conn[lid];
    os << lid << " ";
    os << loop.polylines_.size() << " ";
    for (auto pid : loop.polylines_) {
      os << pid << " ";
    }
    os << std::endl;
  }

  os << obj.faces.size() << std::endl;
  for(auto fid : obj.faces) {
    auto face = obj.fconn[fid];
    auto plane = obj.planes[fid];
    os << fid << " " << face.group_id_ << " " << face.outer_loop_ << std::endl;
    os << face.hole_loops_.size() << " ";
    for(auto lid : face.hole_loops_)
      os << lid << " ";
    os << std::endl;
    os <<  plane.a() << " " << plane.b() << " " << plane.c() << " " << plane.d() << std::endl;
    auto normal = plane.orthogonal_vector();
    normal /= std::sqrt(CGAL::to_double(normal.squared_length()));
#ifdef __DEBUG_
    auto length = std::sqrt(CGAL::to_double(normal.squared_length()));
    os << "normal length: " << length << std::endl;
#endif // __DEBUG__
    os << normal.x() << " " << normal.y() << " " << normal.z() << std::endl;
  }
};

} // namespace IO
} // namespace SBREP

#endif /* SBREP_H */
