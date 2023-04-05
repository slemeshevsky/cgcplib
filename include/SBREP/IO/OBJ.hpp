#ifndef OBJ_H
#define OBJ_H

#include <CGAL/IO/OBJ.h>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

#include <CGAL/Timer.h>

#include <SBREP/Core.hpp>
#include <SBREP/config.hpp>

namespace SBREP {
namespace IO {
/**
 *  \brief Чтение obj-файла
 *
 *  Чтение трехмерного объекта из файла в формате Wavefront OBJ
 *
 *  \param[in] file_path --- путь к файлу
 *  \param[out] mesh --- полигональная сетка
 *  \param[out] fgm --- отображение, где ключ --- индекс треугольника, значение
 * --- номер группы \param[out] num_groups --- количество групп в файле \return
 * удалось ли создать сетку
 */
static int read_OBJ(const std::string file_path, Polygon_mesh &mesh,
                    Face_group_map &fgm, size_type &num_groups) {

  std::string _line;
  int _num_vertices = 0;
  int _num_facets = 0;
  num_groups = 0;
  Polygon_mesh pm;

  if (!CGAL::IO::read_OBJ(file_path, pm)) {
    std::cout << "Unable to open file!!!" << std::endl;
    return 0;
  }

  mesh = pm;
  std::ifstream _file(file_path);
  if (!_file.is_open()) {
    std::cout << "Unable to open file!!!" << std::endl;
    return EXIT_FAILURE;
  }
  while (_file.good()) {
    std::getline(_file, _line);
    if (_line.size() > 1) {
      if (_line[0] == 'v' && _line[1] == ' ') {
        ++_num_vertices;
      }
      if (_line[0] == 'f' && _line[1] == ' ') {
        auto fd = face_descriptor(static_cast<size_type>(_num_facets));
        fgm[fd] = num_groups;
        ++_num_facets;
      }
      if (_line[0] == 'g' && _line[1] == ' ') {
        ++num_groups;
      }
    }
  }
#ifdef __DEBUG__
  std::cout << "* Number of vertices: " << _num_vertices << std::endl;
  std::cout << "* Number of faces: " << _num_facets << std::endl;
  std::cout << "* Number of groups: " << num_groups << std::endl;
#endif // __DEBUG__

#ifdef __DEBUG_
  for (auto &fd : fgm)
    std::cout << "fd: " << fd.first << " group_id: " << fd.second << std::endl;
#endif // __DEBUG__

  _file.close();

  return 1;
};

/**
 *  \brief Запись obj-файла
 *
 *  Сохранение сетки с разбиением элементов поверхностной сетки (например,
 *  треугольников) на группы в файл формата Wavefront OBJ
 *
 *  \param file_path - путь к файлу
 *  \param mesh - полигональная сетка
 *  \param groups - словарь с разбиением элементов сетки на группы
 *  \return return int EXIT_SUCCESS
 */
static int write_OBJ(std::string file_path, Polygon_mesh mesh,
                     Face_group_map face_group_map) {
  std::ofstream out(file_path);

  std::map<vertex_descriptor, unsigned> vertex_map;
  unsigned count = 1;
  for (auto v : mesh.vertices()) {
    auto point = mesh.point(v);
    vertex_map[v] = count;
    ++count;
    out << "v " << point << std::endl;
  }

  size_type _group_id(0);
  while (!face_group_map.empty()) {
    out << "g " << _group_id << std::endl;
    std::vector<face_descriptor> keys;
    for (auto it = face_group_map.begin(); it != face_group_map.end(); it++) {
      if (it->second == _group_id) {
        keys.push_back(it->first);
      }
    }
    if (!keys.empty()) {
      for (auto fd : keys) {
        out << "f";
        for (vertex_descriptor vd :
             vertices_around_face(mesh.halfedge(fd), mesh)) {
          out << " " << vertex_map[vd];
        }
        out << std::endl;
      }
      _group_id++;
    } else {
      for (auto fd : face_group_map) {
        out << "f";
        for (vertex_descriptor vd :
             vertices_around_face(mesh.halfedge(fd.first), mesh)) {
          out << " " << vertex_map[vd];
        }
        out << std::endl;
      }
      auto bi = face_group_map.begin();
      auto ei = face_group_map.end();
      face_group_map.erase(bi, ei);
    }
  }
  out.close();
  return EXIT_SUCCESS;
}

/**
 *  \brief Запись obj-файла
 *
 *  Сохранение сетки с разбиением элементов поверхностной сетки (например,
 *  треугольников) на группы в файл формата Wavefront OBJ
 *
 *  \param file_path - путь к файлу
 *  \param mesh - полигональная сетка
 *  \param groups - словарь с разбиением элементов сетки на группы
 *  \return return int EXIT_SUCCESS
 */
static int write_OBJ(std::string file_path, Polygon_mesh mesh,
                     Regions regions) {
  std::ofstream out(file_path);

  std::map<vertex_descriptor, unsigned> vertex_map;
  unsigned count = 1;
  for (auto v : mesh.vertices()) {
    auto point = mesh.point(v);
    vertex_map[v] = count;
    ++count;
    out << "v " << point << std::endl;
  }
  auto reg_id = 0;
  for (auto region : regions) {
    out << "g " << reg_id++ << std::endl;
    for (auto index : region) {
      out << "f";
      auto _vertices = vertices_around_face(
          mesh.halfedge(face_descriptor(static_cast<size_type>(index))), mesh);
      for (vertex_descriptor vd : _vertices) {
        out << " " << vertex_map[vd];
      }
      out << std::endl;
    }
  }
  out.close();
  return EXIT_SUCCESS;
}
} // namespace IO
} // namespace SBREP

#endif /* OBJ_H */
