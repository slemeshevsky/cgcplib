#include <SBREP/Core.hpp>
#include <SBREP/IO/OBJ.hpp>

#include <gtest/gtest.h>

namespace SBREP {
  //namespace IO {
namespace {

TEST(read_OBJ_test, read_cube) {
  const std::string filename("../../data/frostcube.obj");
  Face_group_map expected_groups;
  for (auto idx = 0; idx < 12; idx++) {
    auto fd = face_descriptor(static_cast<size_type>(idx));
    auto val = static_cast<size_type>(0);
    expected_groups[fd] = val;
  };

  Polygon_mesh pm;
  Face_group_map groups;
  size_type num_groups;
  IO::read_OBJ(filename, pm, groups, num_groups);

  EXPECT_EQ(pm.number_of_vertices(), 8)
      << "Return " << pm.number_of_vertices() << " of vertices: ";
  EXPECT_EQ(pm.number_of_faces(), 12);
  EXPECT_EQ(num_groups, 0);
  EXPECT_EQ(groups, expected_groups);
}

TEST(read_OBJ_test, read_cube_with_groups) {
  const std::string filename("../../data/cubeWithGroup.obj");
  Face_group_map expected_groups;
  for (int idx = 0; idx < 12; idx++) {
    auto fd = face_descriptor(static_cast<size_type>(idx));
    auto val = static_cast<size_type>(idx / int(2) + 1);
    expected_groups[fd] = val;
  };

  Polygon_mesh pm;
  Face_group_map groups;
  size_type num_groups;
  IO::read_OBJ(filename, pm, groups, num_groups);

  EXPECT_EQ(pm.number_of_vertices(), 8)
      << "Return " << pm.number_of_vertices() << " of vertices: ";
  EXPECT_EQ(pm.number_of_faces(), 12);
  EXPECT_EQ(num_groups, 6);
  EXPECT_EQ(groups, expected_groups);
}

TEST(read_OBJ_test, read_with_groups) {
  const std::string filename("../../data/testmesh.obj");
  Face_group_map expected_groups;
  for (int idx = 0; idx < 126; idx++) {
    auto fd = face_descriptor(static_cast<size_type>(idx));
    auto val = idx < 112 ? static_cast<size_type>(0) : static_cast<size_type>(1);
    expected_groups[fd] = val;
  };
  
  Polygon_mesh pm;
  Face_group_map groups;
  size_type num_groups;
  IO::read_OBJ(filename, pm, groups, num_groups);

  EXPECT_EQ(pm.number_of_vertices(), 65)
      << "Return " << pm.number_of_vertices() << " of vertices: ";
  EXPECT_EQ(pm.number_of_faces(), 126)
      << "Return " << pm.number_of_faces() << " of vertices: ";
  EXPECT_EQ(num_groups, 1);
  //  EXPECT_EQ(groups, expected_groups);
}
  
TEST(read_OBJ_test, read_with_groups_1) {
  const std::string filename("../../data/cubeRemeshedInfluenseOnFaces.obj");
  Face_group_map expected_groups;
  for (int idx = 0; idx < 126; idx++) {
    auto fd = face_descriptor(static_cast<size_type>(idx));
    auto val = idx < 112 ? static_cast<size_type>(0) : static_cast<size_type>(1);
    expected_groups[fd] = val;
  };

  Polygon_mesh pm;
  Face_group_map groups;
  size_type num_groups;
  IO::read_OBJ(filename, pm, groups, num_groups);

  EXPECT_EQ(pm.number_of_vertices(), 347)
      << "Return " << pm.number_of_vertices() << " of vertices: ";
  EXPECT_EQ(pm.number_of_faces(), 690)
      << "Return " << pm.number_of_faces() << " of vertices: ";
  EXPECT_EQ(num_groups, 7);
}
} // namespace
  //} // namespace IO
} // namespace SBREP
