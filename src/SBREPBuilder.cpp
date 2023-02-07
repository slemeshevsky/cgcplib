#include <cgcplib/SBREPBuilder.hpp>

namespace cgcp::SBREP
{
    Polygon_mesh SBREPBuilder::ReadObjFile(std::string obj_file_path)
    {
        std::string filename = CGAL::data_file_path(obj_file_path);
        Polygon_mesh *mesh;
        if (!CGAL::IO::read_polygon_mesh(filename, mesh))
        {
            std::cerr << "Invalid input file." << std::endl;
            return EXIT_FAILURE;
        };
        return mesh;
    }
    SBREPBuilder SBREPBuilder::Convert(Polygon_mesh polygon_mesh)
    {
        const Face_range face_range = faces(polygon_mesh);

        // Создаем экземпляры классов Neighbor_query и Regin_type
        Neighbor_query neighbor_query(polygon_mesh);
        const Vertex_to_point_map vertex_to_point_map(get(CGAL::vertex_point, polygon_mesh));
        Region_type region_type(
            polygon_mesh,
            max_distance_to_plane, max_accepted_angle, min_region_size,
            vertex_to_point_map);

        // Сортируем индексы треугольников
        Sorting sorting(
            polygon_mesh, neighbor_query,
            vertex_to_point_map);
        sorting.sort();

        // Создаем экземпляр класса, реализующего алгоритм растущего региона
        Region_growing region_growing(
            face_range, neighbor_query, region_type,
            sorting.seed_map());
        // Запускаем алгоритм для построения плоскостей
        Regions regions;
        region_growing.detect(std::back_inserter(regions));

        return nullptr;
    };
};
