#include <CGAL/IO/Color.h>
#include <CGAL/Kernel/global_functions_3.h>
#include <CGAL/Timer.h>
#include <CGAL/exceptions.h>

#include <SBREP/IO/OBJ.hpp>
#include <SBREP/SBREP_builder.hpp>
#include <algorithm>
#include <cstddef>
#include <exception>
#include <iostream>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

#include "SBREP/Core.hpp"
#include "SBREP/SBREP_object.hpp"

namespace SBREP {

std::pair<Regions, Face_group_map>
SBREP_builder::Group_triangles_by_plane(Polygon_mesh polygon_mesh) {
  const Face_range face_range = faces(polygon_mesh);

  // Создаем экземпляры классов Neighbor_query и Region_type
  Neighbor_query neighbor_query(polygon_mesh);
  const Vertex_to_point_map vertex_to_point_map(
      get(CGAL::vertex_point, polygon_mesh));
  Region_type region_type(polygon_mesh, max_distance_to_plane,
                          max_accepted_angle, min_region_size,
                          vertex_to_point_map);

  // Сортируем индексы треугольников
  Sorting sorting(polygon_mesh, neighbor_query, vertex_to_point_map);
  sorting.sort();

  // Создаем экземпляр класса, реализующего алгоритм растущего региона
  Region_growing region_growing(face_range, neighbor_query, region_type,
                                sorting.seed_map());
  // Запускаем алгоритм для построения плоскостей
  Regions regions;
  region_growing.detect(std::back_inserter(regions));

#ifdef __DEBUG__
  // Выводим на печать количество найденных подобластей
  std::cout << std::endl
            << "* " << regions.size() << " regions have been found."
            << std::endl;
  // for (auto i = 0; i < regions.size(); i++) {
  //   std::cout << "* Region: " << i << std::endl;
  //   for (auto fd : regions[i])
  //     std::cout << "\t fd: " << fd << std::endl;
  // }
#endif // __DEBUG__
  Face_group_map face_plane_map;
  for (auto i = 0; i < regions.size(); i++) {
    for (auto fi : regions[i]) {
      auto fd = face_descriptor(static_cast<size_type>(fi));
      face_plane_map[fd] = i;
    }
  }
  return std::pair<Regions, Face_group_map>(regions, face_plane_map);
}

std::tuple<Halfedges, Vertex_halfedges_map, vertex_descriptor>
SBREP_builder::Initialize_ds(Polygon_mesh mesh, Region region) {
  Halfedges HE;
  Vertex_halfedges_map VE;
  face_descriptor f(static_cast<size_type>(region[0]));
  vertex_descriptor v_x = mesh.source(mesh.halfedge(f));

  for (auto idx : region) {
    face_descriptor fd(static_cast<size_type>(idx));
    auto he = mesh.halfedge(fd);
    for (auto i = 0; i < 3; i++) {
      auto fi = static_cast<std::size_t>(mesh.face(mesh.opposite(he)));
      auto fi_in_region = std::find(std::begin(region), std::end(region), fi);
      if (fi_in_region == std::end(region)) {
        HE.push_back(he);
        auto vd = mesh.source(he);
        if (mesh.point(vd) >= mesh.point(v_x))
          v_x = vd;
        VE[vd].push_back(he);
      }
      he = mesh.next(he);
    }
  }
  auto res = std::tuple<Halfedges, Vertex_halfedges_map, vertex_descriptor>(
      HE, VE, v_x);
#ifdef __DEBUG__
  std::cout << "* DS for region: " << 0 << " intialized" << std::endl;
#endif // __DEBUG__
  return res;
}

/**
 *  \brief Получение внешней границы области
 *
 *  Вычисляет внешнюю границу (петлю) подобласти.
 *
 *  \param mesh --- входная триангуляция поверхности (Polygon_mesh).
 *  \param HE --- список граничных ребер для группы треугольников
 *  \param PtE --- отображение (SBREP::Vertex_halfedges_map) точек,
 *  у которого ключ --- индекс точки, а значение --- вектор искходящих
 *  из данной точки граничных ребер,
 *  \param he --- стартовое ребро (SBREP::Halfedges),
 *  \param vd --- стартовая точка (SBREP::vertex_descriptor).
 *  \return вектор (SBREP::vertex_descriptor) с вершинами составляющими
 * внешнюю границу.
 */
std::vector<vertex_descriptor>
SBREP_builder::Extract_linear_ring(Vertex_halfedges_map PtE,
                                   vertex_descriptor vd, Halfedges &HE,
                                   Polygon_mesh &mesh) {
  std::vector<vertex_descriptor> lr{vd};
  auto halfedges = PtE[vd];
  auto he = halfedges[0];

#ifdef __DEBUG__
  std::cout << "*** Start extraction of outer boundary ***" << std::endl;
#endif // __DEBUG__

  while (true) {
    HE.erase(std::remove(std::begin(HE), std::end(HE), he), std::end(HE));
    HE.shrink_to_fit();
    auto vi = mesh.target(he);
    lr.push_back(vi);
    if (vi == vd)
      break;
    auto next_edges = PtE[vi];
    he = select_edge(he, next_edges, mesh);
  }
#ifdef __DEBUG__
  std::cout << "*** Outer boundary extracted. ***" << std::endl;
#endif // __DEBUG__
  return lr;
}

std::vector<std::vector<vertex_descriptor>>
SBREP_builder::Extract_holes(Vertex_halfedges_map PtE, Halfedges &HE,
                             Polygon_mesh &mesh) {
  std::vector<std::vector<vertex_descriptor>> HR;
#ifdef __DEBUG__
  std::cout << "*** Start holes extraction ***" << std::endl;
#endif // __DEBUG__
  while (!HE.empty()) {
    auto he = HE[0];
    auto vi = mesh.target(he);
    auto lr = Extract_linear_ring(PtE, vi, HE, mesh);
    HR.push_back(lr);
  }
#ifdef __DEBUG__
  std::cout << "*** " << HR.size() << " holes extracted. ***" << std::endl;
#endif // __DEBUG__
  return HR;
}

Regions SBREP_builder::Group_triangles(Polygon_mesh mesh, Regions planes,
                                       Face_group_map &fgm,
                                       size_type &num_groups) {
#ifdef __DEBUG__
  std::cout << "* Start grouping of triangles." << std::endl;
  std::cout << "Groups number: " << num_groups << std::endl;
  std::cout << "Planes number: " << planes.size() << std::endl;
#endif // __DEBUG__

  auto planes_number = planes.size();
  Regions regions(num_groups + planes_number, Region());

#ifdef __DEBUG__
  std::cout << "Regions number: " << regions.size() << std::endl;
#endif // __DEBUG__

#ifdef __DEBUG__
  for (auto face : fgm)
    std::cout << "fd: " << face.first << " group_id: " << face.second
              << std::endl;
  std::cout << "***** Start corrections *********" << std::endl;
#endif // __DEBUG__

  for (auto i = 0; i < planes_number; i++) {
    Region region;
    for (auto idx : planes[i]) {
      face_descriptor fd(static_cast<size_type>(idx));

#ifdef __DEBUG__
      std::cout << "fd: " << fd << " group_id: " << fgm[fd] << std::endl;
#endif // __DEBUG__
      if (fgm[fd] > 0)
        fgm[fd] += planes_number - 1;
      else
        fgm[fd] = i;
#ifdef __DEBUG__
      std::cout << "Corrected fd: " << fd << " group_id: " << fgm[fd]
                << std::endl;
#endif // __DEBUG__
    }
  }

#ifdef __DEBUG__
  std::cout << "face_group_map corrected!" << std::endl;
  std::cout << "fgm size: " << fgm.size() << std::endl;
#endif // __DEBUG__

  for (auto face : fgm) {
    auto face_id = face.first;
    auto group_id = face.second;
#ifdef __DEBUG__
    std::cout << "face_id: " << face_id << " group_id: " << group_id
              << std::endl;
#endif // __DEBUG__
    regions[group_id].push_back((size_type)face_id);
  }

#ifdef __DEBUG__
  std::cout << "regions updated!" << std::endl;
  size_type idx(0);
  for (auto reg : regions) {
    std::cout << "* Region: " << idx++ << std::endl;
    for (auto fd : reg)
      std::cout << "\t fd: " << fd << std::endl;
  }
#endif // __DEBUG__

  return regions;
}

Plane_3 SBREP_builder::Get_plane(Region region, Polygon_mesh &mesh) {
  auto fd = face_descriptor(static_cast<size_type>(region[0]));
  std::vector<Point_3> points;
  for (auto vd : vertices_around_face(mesh.halfedge(fd), mesh))
    points.push_back(mesh.point(vd));
  return Plane_3(points[0], points[1], points[2]);
}

int SBREP_builder::Get_group_id(Region region, Polygon_mesh &mesh,
                                Face_group_map &fgm) {
  auto fd = face_descriptor(static_cast<size_type>(region[0]));
  auto group_id = fgm[fd] - 1;
  return group_id;
}

std::pair<Regions, std::vector<std::size_t>>
SBREP_builder::Group_triangles_by_group(Region planar_group,
                                        Face_group_map &fgm) {
  auto res = Regions();
  auto groups_set = std::set<size_type>();

  //  auto num_groups = 0;
  for (const auto &fid : planar_group) {
    auto fd = face_descriptor(static_cast<size_type>(fid));
    groups_set.insert(fgm[fd]);

#ifdef __DEBUG_
    std::cout << "Group_id: " << fgm[fd] << std::endl;
#endif // __DEBUG__
  }
  auto num_groups = groups_set.size();
  std::vector<std::size_t> groups_id(groups_set.begin(), groups_set.end());
  res.resize(num_groups);
  for (const auto &fid : planar_group) {
    auto fd = face_descriptor(static_cast<size_type>(fid));
    auto value = fgm[fd];
    auto value_in_group = std::find(groups_id.begin(), groups_id.end(), value);
    auto i = value_in_group - groups_id.begin();
    if (value_in_group != groups_id.end())
      res[i].push_back(static_cast<std::size_t>(fid));
  }

  return std::pair<Regions, std::vector<std::size_t>>(res, groups_id);
}

std::map<std::pair<int, int>, std::vector<Edge_idx>>
SBREP_builder::Group_edges_by_groups(std::set<Edge_idx> eids, Polygon_mesh &pm,
                                     Face_group_map &face_group_map,
                                     SBrep &obj) {
  auto res = std::map<std::pair<int, int>, std::vector<Edge_idx>>();
  for (auto e : eids) {
    auto v0 =
        vertex_descriptor(static_cast<size_type>(obj.econn[e].vertices_.first));
    auto v1 = vertex_descriptor(
        static_cast<size_type>(obj.econn[e].vertices_.second));
    auto he = pm.halfedge(v0, v1);
    auto fd = pm.face(he);
    auto fd_opp = pm.face(pm.opposite(he));
    int group0 = face_group_map[fd];     // + face_plane_map[fd];
    int group1 = face_group_map[fd_opp]; // + face_plane_map[fd_opp];
    std::pair<int, int> key;
    //    if (plane0 == plane1)
    key = (group0 < group1) ? std::pair<int, int>(group0, group1)
                            : std::pair<int, int>(group1, group0);
    // else if (group0 == group1) {
    //   key = (plane0 < plane1) ? std::pair<int, int>(plane0, plane1)
    //                           : std::pair<int, int>(plane1, plane0);

    res[key].push_back(e);
    //}
  }
  return res;
}

std::map<int, std::vector<Edge_idx>>
SBREP_builder::Group_edges_by_connection(std::vector<Edge_idx> edges,
                                         SBrep &obj) {
  auto res = std::map<int, std::vector<Edge_idx>>();
  int key{0};
  auto e = edges.front();
  res[key].push_back(e);
  while (!edges.empty()) {
    //    while(true) {
    res[key].push_back(e);
    edges.erase(std::remove(std::begin(edges), std::end(edges), e));
    const auto &[e0, e1] = obj.get_adjacent_edges(e);
    auto e0_in_edges = std::find(std::begin(edges), std::end(edges), e0);
    auto e1_in_edges = std::find(std::begin(edges), std::end(edges), e1);
    if ((e0_in_edges == std::end(edges)) && (e1_in_edges == std::end(edges))) {
      key++;
      e = edges.front();
    }
    else if (e0_in_edges == std::end(edges)) {
      e = *e1_in_edges;
    }
    else {
      e = *e1_in_edges;
    }
    //    }
  }
#ifdef __DEBUG__
  std::cout << "Group by connection: ";

  for (auto &[k, value] : res) {
    std::cout << "(" << k << "): ";
    for(auto v : value)
      std::cout << v << " ";
    std::cout << std::endl;
  }
  std::cout << std::endl;
#endif // __DEBUG__
  return res;
}

SBrep SBREP_builder::Convert(Polygon_mesh polygon_mesh, Face_group_map fgm,
                             size_type num_groups) {
  SBrep res;
  auto [planar_groups, fpm] = Group_triangles_by_plane(polygon_mesh);
  res.groups = planar_groups;
  res.pm = polygon_mesh;
  //  auto groups = Group_triangles(polygon_mesh, planar_groups, fgm,
  //  num_groups);

#ifdef __DEBUG_
  std::cout << "Planar groups size: " << planar_groups.size() << std::endl;
#endif // __DEBUG__

  auto reg_id = 0;
  auto planar_group_id = 0;
  Regions regions;

  for (const auto &planar_group : planar_groups) {
    
#ifdef __DEBUG__
    std::cout << "Planar group: " << planar_group_id++ << std::endl;
#endif // __DEBUG__

    auto [subregions, groups_id] = Group_triangles_by_group(planar_group, fgm);

#ifdef __DEBUG__
    std::cout << "Number of subregionds: " << subregions.size() << std::endl;
#endif // __DEBUG__

    for (auto &region : subregions) {
      auto group_id = Get_group_id(region, polygon_mesh, fgm);
      auto [HE, VE, vx] = Initialize_ds(polygon_mesh, region);
      auto outer_bound = Extract_linear_ring(VE, vx, HE, polygon_mesh);
      auto holes = Extract_holes(VE, HE, polygon_mesh);
      for (auto vd : outer_bound) {
        res.add_vertex(polygon_mesh.point(vd), vd);
      }
      const auto eids = add_edges(outer_bound, polygon_mesh, fgm, fpm, res);
      auto group_edges =
          Group_edges_by_groups(eids, polygon_mesh, fgm, res);
      if(group_edges.size() == 1)
        group_edges = Group_edges_by_groups(eids, polygon_mesh, fpm, res);
#ifdef __DEBUG__

      std::cout << "Groups: " << std::endl;
      for (auto &[key, val] : group_edges) {
        std::cout << "(" << key.first << ", " << key.second << "): ";
        for (auto e : val)
          std::cout << e << " (" << res.econn[e].vertices_.first << ", "
                    << res.econn[e].vertices_.second << ") ";
        std::cout << std::endl;
        //Group_edges_by_connection(val, res);
      }

#endif // __DEBUG__
      auto polylines = add_outer_polylines(group_edges, res);
#ifdef __DEBUG__

      std::cout << "Polylines: " << std::endl;
      for (auto pid : polylines) {
        std::cout << "(" << pid << "): ";
        for (auto e : res.poly_conn[pid].edges_) {
          std::cout << res.econn[e].vertices_.first << " ";
        }
      }
      std::cout << std::endl;

#endif // __DEBUG__
      auto outer_loop = res.add_loop(polylines);
#ifdef __DEBUG_

      res.print_loop(outer_loop);

#endif // __DEBUG__

      auto hole_loops = std::vector<Loop_idx>();
      for (auto hole : holes) {
        for (auto vd : hole) {
          res.add_vertex(polygon_mesh.point(vd), vd);
        }
        const auto heids = add_edges(hole, polygon_mesh, fgm, fpm, res);
        const auto v_heids = std::vector<Edge_idx>(heids.begin(), heids.end());
        auto pid = res.add_polyline(v_heids);
        std::vector<Polyline_idx> hole_polylines{pid};
        auto lid = res.add_loop(hole_polylines);
        hole_loops.push_back(lid);
      }
      auto fid = res.add_face(outer_loop, hole_loops, group_id);
      auto plane = Get_plane(region, polygon_mesh);
      res.planes[fid] = plane;
      reg_id++;
    }
  }
  return res;
}

std::set<Edge_idx> SBREP_builder::add_edges(std::vector<vertex_descriptor> ring,
                                            Polygon_mesh &pm,
                                            Face_group_map &fgm,
                                            Face_group_map &fpm, SBrep &obj) {
  auto eids = std::set<Edge_idx>();
  for (auto v = ring.begin(); v != ring.end() - 1; v++) {
    auto v0 = static_cast<Vertex_idx>(*v);
    auto v1 = static_cast<Vertex_idx>(*(v + 1));

    auto he = pm.halfedge(*v, *(v + 1));
    auto g0 = fgm[pm.face(he)];
    auto g1 = fgm[pm.face(pm.opposite(he))];
    auto p0 = fpm[pm.face(he)];
    auto p1 = fpm[pm.face(pm.opposite(he))];
    auto eid = obj.add_edge(v0, v1, g0, g1, p0, p1);
    eids.insert(eid);
  }

#ifdef __DEBUG_
  for (auto e : obj.edges) {
    std::cout << "Edge: " << e << std::endl;
    std::cout << "\t Vertices: " << obj.econn[e].vertices_.first << " "
              << obj.econn[e].vertices_.second << std::endl;
    std::cout << "\t Groups: " << obj.econn[e].groups_.first << " "
              << obj.econn[e].groups_.second << std::endl;
    std::cout << "\t Planes: " << obj.econn[e].planes_.first << " "
              << obj.econn[e].planes_.second << std::endl;
  }
#endif // __DEBUG__
  return eids;
}

std::vector<Edge_idx>::iterator
SBREP_builder::find_connected_component(std::vector<Edge_idx>::iterator first,
                                        std::vector<Edge_idx>::iterator last,
                                        SBrep &obj) {
  auto it = first;
  while (it != last) {
    auto current = *(it);
    auto next = *(it + 1);
    auto v0 = obj.econn[current].vertices_.second;
    auto v1 = obj.econn[next].vertices_.first;
    if (v0 != v1)
      break;
    ++it;
  }

  std::cout << "Index: " << std::distance(first, it) << std::endl;

  return it;
}

std::vector<Polyline_idx> SBREP_builder::add_outer_polylines(
    std::map<std::pair<int, int>, std::vector<Edge_idx>> grouped_edges,
    SBrep &obj) {
  auto polylines = std::vector<Polyline_idx>();
  for (auto [key, eids] : grouped_edges) {

#ifdef __DEBUG__

    std::cout << "**************************************************"
              << std::endl;
    std::cout << "Grouped edges: " << std::endl;
    for (auto e : eids)
      std::cout << "(" << e << "): " << obj.econn[e].vertices_.first << " "
                << obj.econn[e].vertices_.second << std::endl;
    std::cout << "**************************************************"
              << std::endl;
    auto it = find_connected_component(eids.begin(), eids.end(), obj);
    std::cout << "Is one connected component? " << (it == eids.end())
              << " ( idx: " << *(it) << ") " << std::endl;

#endif // __DEBUG__

    auto pid = obj.add_polyline(eids);
    polylines.push_back(pid);

    //     auto edges_it = eids.begin();
    //     while (edges_it < eids.end()) {
    // #ifdef __DEBUG__

    //       std::cout << "Edges_it idx: " << *(edges_it) << std::endl;

    // #endif // __DEBUG__
    //       const auto &[begin, end] = find_connected_component(eids, obj);
    // #ifdef __DEBUG__

    //       std::cout << "Begin idx: " << *(begin) << "; End idx: " << *(end)
    //                 << std::endl;

    // #endif // __DEBUG__

    //       auto polyline = std::vector<Edge_idx>();
    //       for (auto it = begin; it <= end; it++) {
    //         polyline.push_back(*(it));
    //       }
    //       auto pid = obj.add_polyline(polyline);
    //       polylines.push_back(pid);
    //       eids.erase(begin, end+1);
    //       edges_it = eids.begin();

    // #ifdef __DEBUG__

    //       std::cout << "Edges_it idx: " << *(edges_it) << std::endl;

    // #endif // __DEBUG__

    //   }
  }
  return polylines;
}

// std::vector<Polyline_idx>
// SBREP_builder::add_outer_polylines(std::set<Edge_idx> eids, SBrep &obj) {
// #ifdef __DEBUG_
//   for (auto e : eids) {
//     std::cout << "Edge: " << e << std::endl;
//     std::cout << "\t Vertices: " << obj.econn[e].vertices_.first << " "
//               << obj.econn[e].vertices_.second << std::endl;
//     std::cout << "\t Groups: " << obj.econn[e].groups_.first << " "
//               << obj.econn[e].groups_.second << std::endl;
//     std::cout << "\t Planes: " << obj.econn[e].planes_.first << " "
//               << obj.econn[e].planes_.second << std::endl;
//   }
// #endif // __DEBUG__
//   auto polylines = std::vector<Polyline_idx>();
//   auto v_eids = std::vector<Edge_idx>(eids.begin(), eids.end());
//   auto &[gr0, gr1] = obj.econn[v_eids[0]].groups_;
//   auto &[pl0, pl1] = obj.econn[v_eids[0]].planes_;
//   auto &[gr0_next, gr1_next] = obj.econn[v_eids[1]].groups_;
//   auto &[pl0_next, pl1_next] = obj.econn[v_eids[1]].planes_;
//   auto poly_bounds = std::vector<Edge_idx>();
//   auto ccw = (pl0 == pl0_next);
//   poly_bounds.push_back(v_eids[0]);
//   if (!ccw)
//     v_eids = std::vector<Edge_idx>(v_eids.rbegin(), v_eids.rend());
//   for (auto it = v_eids.begin(); it < v_eids.end(); it++) {
//     auto e = obj.econn[*(it)];
//     auto next_e = obj.econn[*(it + 1)];
//     auto cur_pl1 = e.planes_.second;
//     auto next_pl1 = next_e.planes_.second;
//     auto cur_g1 = e.groups_.second;
//     auto next_g1 = next_e.groups_.second;
//     if ((cur_pl1 == next_pl1) && (cur_g1 == next_g1))
//       continue;
//     else
//       poly_bounds.push_back(*(it + 1));
//   }
//   if (poly_bounds.size() == 1) {
//     auto pid = obj.add_polyline(v_eids);
//     polylines.push_back(pid);
//   } else {
//     for (auto it = poly_bounds.begin(); it < poly_bounds.end() - 1; it++) {
//       auto polyline = std::vector<Edge_idx>();
//       auto bit = std::find(v_eids.begin(), v_eids.end(), *(it));
//       auto eit = std::find(v_eids.begin(), v_eids.end(), *(it + 1));
//       for (; bit < eit; bit++)
//         polyline.push_back(*bit);
//       auto pid = obj.add_polyline(polyline);
//       polylines.push_back(pid);
//     }
//   }

//   return polylines;
// }

// std::vector<Polyline_idx>
// SBREP_builder::add_outer_polylines(std::set<Edge_idx> eids,
//                                    std::size_t num_planes, SBrep &obj) {
// #ifdef __DEBUG_
//   std::cout << "********* Edges in outer_bound: *********" << std::endl;
//   for (auto eid : eids) {
//     std::cout << eid << " verts: ";
//     std::cout << obj.econn[eid].vertices_.first << ", "
//               << obj.econn[eid].vertices_.second << " groups: ";
//     std::cout << obj.econn[eid].groups_.first << " "
//               << obj.econn[eid].groups_.second << std::endl;
//   }
//   std::cout << "*****************************************" << std::endl;
// #endif // __DEBUG__

//   auto polylines = std::vector<Polyline_idx>();
//   for (auto plane_id = 0; plane_id < num_planes; plane_id++) {
//     std::vector<Edge_idx> es;
//     for (auto eid : eids) {
//       if ((obj.econn[eid].groups_.first == plane_id) ||
//           (obj.econn[eid].groups_.second == plane_id))
//         es.push_back(eid);
//     }
//     for (auto plane_id_new = 0; plane_id_new < num_planes; plane_id_new++)
//     {
//       auto polyline = std::vector<Edge_idx>();
//       if (plane_id_new != plane_id) {
//         for (const auto &eid : es) {
//           if ((obj.econn[eid].groups_.first == plane_id_new) ||
//               (obj.econn[eid].groups_.second == plane_id_new))
//             polyline.push_back(eid);
//         }
//       } else {
//       }

//       if (!polyline.empty()) {
//         auto poly_size = obj.polylines.size();
//         auto pid = obj.add_polyline(polyline);
//         auto pid_in_polylines =
//             std::find(polylines.begin(), polylines.end(), pid);
//         if (pid_in_polylines == polylines.end())
//           polylines.push_back(pid);
//       }
//     }
//   }
//   if (polylines.empty()) {
//     auto polyline = std::vector<Edge_idx>(eids.begin(), eids.end());
//     auto pid = obj.add_polyline(polyline);
//     polylines.push_back(pid);
//   }
//   return polylines;
// }

halfedge_descriptor SBREP_builder::select_edge(halfedge_descriptor he,
                                               Halfedges halfedges,
                                               Polygon_mesh &mesh) {
  auto res_he = halfedges[0];
  auto p = mesh.point(mesh.source(he));
  auto q = mesh.point(mesh.target(he));
  for (auto hei : halfedges) {
    auto r = mesh.point(mesh.target(res_he));
    auto angle1 = CGAL::approximate_angle(p, q, r);
    r = mesh.point(mesh.target(hei));
    auto angle2 = CGAL::approximate_angle(p, q, r);
    if (angle1 <= angle2)
      res_he = hei;
  }
  return res_he;
}
} // namespace SBREP
