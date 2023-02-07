#include <CGAL/IO/Color.h>

#include <cgcplib/SBREPBuilder.hpp>
#include <cgcplib/SBREPObject.hpp>
#include <string>

// void SBREPBuilder::init(double max_d, double max_a, int min_size) {
//   max_distance_to_plane = FT(max_d);
//   max_accepted_angle = FT(max_a);
//   min_region_size = min_size;
// };

// const FT SBREPBuilder::max_distance_to_plane = FT(1);
// const FT SBREPBuilder::max_accepted_angle = FT(45);
// const std::size_t SBREPBuilder::min_region_size = 5;

using Color = CGAL::IO::Color;

Polygon_mesh SBREPBuilder::ReadObjFile(std::string obj_file_path) {
  std::string filename = CGAL::data_file_path(obj_file_path);
  Polygon_mesh mesh;
  if (!CGAL::IO::read_polygon_mesh(filename, mesh)) {
    std::cerr << "Invalid input file." << std::endl;
  };
  return mesh;
};

Regions SBREPBuilder::Convert(Polygon_mesh polygon_mesh) {
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

  // Print the number of found regions.
  std::cout << "* " << regions.size() << " regions have been found"
            << std::endl;
  return regions;
}

int SBREPBuilder::SaveRegions(Polygon_mesh polygon_mesh, Regions regions,
                              std::string path, std::string extension="off") {
  using Face_index = typename Polygon_mesh::Face_index;

  srand(static_cast<unsigned int>(time(nullptr)));
  bool created;
  typename Polygon_mesh::template Property_map<Face_index, Color> face_color;
  boost::tie(face_color, created) =
      polygon_mesh.template add_property_map<Face_index, Color>("f:color",
                                                                Color(0, 0, 0));
  if (!created) {
    std::cout << std::endl
              << "region_growing_on_polygon_mesh example finished" << std::endl
              << std::endl;
    return EXIT_FAILURE;
  }

  path = path + "." + extension;
  std::ofstream out(path);
  // Iterate through all regions.
  for (const auto& region : regions) {
    // Generate a random color.
    const Color color(static_cast<unsigned char>(rand() % 256),
                      static_cast<unsigned char>(rand() % 256),
                      static_cast<unsigned char>(rand() % 256));
    // Iterate through all region items.
    using size_type = typename Polygon_mesh::size_type;
    for (const auto index : region)
      face_color[Face_index(static_cast<size_type>(index))] = color;
  }
  // 

  if (extension == "off")
    out << polygon_mesh;
  else if (extension == "obj")
    CGAL::IO::write_OBJ(out, polygon_mesh);
  else if (extension == "ply")
    CGAL::IO::write_PLY(out, polygon_mesh);
  else
    std::cout <<"* output file format does not support." << std::endl;
  out.close();
  std::cout << "* polygon mesh is saved in " << path << std::endl;

  return EXIT_SUCCESS;
}
