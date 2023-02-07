#include <cgcplib/SBREPObject.hpp>
#include <cgcplib/SBREPBuilder.hpp>
#include <string>

// void SBREPBuilder::init(double max_d, double max_a, int min_size) {
//   max_distance_to_plane = FT(max_d);
//   max_accepted_angle = FT(max_a);
//   min_region_size = min_size;
// };

const FT SBREPBuilder::max_distance_to_plane = FT(1);
const FT SBREPBuilder::max_accepted_angle = FT(45);
const std::size_t SBREPBuilder::min_region_size = 5;

Polygon_mesh SBREPBuilder::ReadObjFile(std::string obj_file_path) {
  std::string filename = CGAL::data_file_path(obj_file_path);
  Polygon_mesh mesh;
  if (!CGAL::IO::read_polygon_mesh(filename, mesh)) {
    std::cerr << "Invalid input file." << std::endl;
  };
  return mesh;
};

int SBREPBuilder::Convert(Polygon_mesh polygon_mesh) {
  const Face_range face_range = faces(polygon_mesh);
  
  // Создаем экземпляры классов Neighbor_query и Regin_type
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

  return EXIT_SUCCESS;
};
