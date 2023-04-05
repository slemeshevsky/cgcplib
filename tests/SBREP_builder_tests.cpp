#include <gtest/gtest.h>
#include <iostream>
#include <string>

#include <SBREP/Core.hpp>
#include <SBREP/IO/OBJ.hpp>
#include <SBREP/SBREP_builder.hpp>

namespace SBREP {
const FT SBREP_builder::max_distance_to_plane = FT(1);
const FT SBREP_builder::max_accepted_angle = FT(1);
const std::size_t SBREP_builder::min_region_size = 1;

namespace {

class SBREP_builder_Test : public ::testing::Test {
protected:
  static void SetUpTestSuite() {
    file_path = "../../data/cubeRemeshedInfluenseOnFaces.obj";
    //    file_path = "../../data/testmesh.obj";
    // IO::read_OBJ(file_path, pm_, fgm_, num_groups_);
    // regions_ = SBREP_builder::Group_triangles_by_plane(pm_);
    // fgm_regs_ =
    //     SBREP_builder::Group_triangles(pm_, regions_, fgm_, num_groups_);
  }
  //  void SetUp() override {}
  static std::string file_path;
  // static Polygon_mesh pm_;
  // static Face_group_map fgm_;
  // static size_type num_groups_;
  // static Regions regions_;
  // static Regions fgm_regs_;
};

// Polygon_mesh SBREP_builder_Test::pm_;
// Face_group_map SBREP_builder_Test::fgm_;
// size_type SBREP_builder_Test::num_groups_ = 0;
// Regions SBREP_builder_Test::regions_;
// Regions SBREP_builder_Test::fgm_regs_;
std::string SBREP_builder_Test::file_path;

TEST_F(SBREP_builder_Test, group_triangles_by_planes_works) {
  Polygon_mesh pm_;
  Face_group_map fgm_;
  //Face_group_map fpm_;
  size_type num_groups_;
  //Regions regions_;

  IO::read_OBJ(file_path, pm_, fgm_, num_groups_);
  const auto &[regions_, fpm_] = SBREP_builder::Group_triangles_by_plane(pm_);

  EXPECT_EQ(num_groups_, 7);
  EXPECT_EQ(regions_.size(), 6);
}

TEST_F(SBREP_builder_Test, group_triangles_by_group) {
  Polygon_mesh pm_;
  Face_group_map fgm_;
  size_type num_groups_;

  IO::read_OBJ(file_path, pm_, fgm_, num_groups_);
  const auto &[regions_, fpm_] = SBREP_builder::Group_triangles_by_plane(pm_);
  const auto &[sub_regions_0, groups_id]= SBREP_builder::Group_triangles_by_group(regions_[4], fgm_);
  
  EXPECT_EQ(sub_regions_0.size(), 2);
  EXPECT_EQ(regions_.size(), 6);
}
  
TEST_F(SBREP_builder_Test, DISABLED_group_triangles_works) {
  Polygon_mesh pm;
  Face_group_map fgm;
  size_type num_groups;
  //Regions planes;
  Regions fgm_regs;

  IO::read_OBJ(file_path, pm, fgm, num_groups);
  const auto &[planes, _] = SBREP_builder::Group_triangles_by_plane(pm);
  fgm_regs = SBREP_builder::Group_triangles(pm, planes, fgm, num_groups);
  EXPECT_EQ(fgm_regs.size(), 7);
}

TEST_F(SBREP_builder_Test, DISABLED_initialize_ds_works_quad) {
  Polygon_mesh pm_;
  Face_group_map fgm_;
  size_type num_groups_;
  Regions fgm_regs;

  IO::read_OBJ(file_path, pm_, fgm_, num_groups_);
  const auto &[regions_, fpm_] = SBREP_builder::Group_triangles_by_plane(pm_);
  const auto &[sub_regions_0, groups_id]= SBREP_builder::Group_triangles_by_group(regions_[4], fgm_);
  
  // for (auto i = 1; i < fgm_regs.size(); i++) {
  //   auto [HE, VE, vx] = SBREP_builder::Initialize_ds(pm_, fgm_regs[i]);
  //   EXPECT_EQ(HE.size(), 8);
  //   for (auto v : VE)
  //     EXPECT_EQ(v.second.size(), 1);
  // }
}

TEST_F(SBREP_builder_Test, DISABLED_initialize_ds_works_with_hole) {
  Polygon_mesh pm;
  Face_group_map fgm;
  size_type num_groups;
  //  Regions planes;
  Regions fgm_regs;

  IO::read_OBJ(file_path, pm, fgm, num_groups);
  const auto &[planes, _] = SBREP_builder::Group_triangles_by_plane(pm);
  fgm_regs = SBREP_builder::Group_triangles(pm, planes, fgm, num_groups);

  auto [HE, VE, vx] = SBREP_builder::Initialize_ds(pm, fgm_regs[0]);
  EXPECT_EQ(HE.size(), 16);
  for (auto v : VE)
    EXPECT_EQ(v.second.size(), 1);
}

TEST_F(SBREP_builder_Test, DISABLED_extract_linear_ring_works_without_hole) {
  Polygon_mesh pm;
  Face_group_map fgm;
  size_type num_groups;
  //  Regions planes;
  Regions fgm_regs;

  IO::read_OBJ(file_path, pm, fgm, num_groups);
  const auto &[planes, _] = SBREP_builder::Group_triangles_by_plane(pm);
  fgm_regs = SBREP_builder::Group_triangles(pm, planes, fgm, num_groups);

  for (auto i = 1; i < fgm_regs.size(); i++) {
    auto [HE, VE, vx] = SBREP_builder::Initialize_ds(pm, fgm_regs[i]);
    auto he = VE[vx];
    EXPECT_EQ(HE.size(), 8);
    auto linear_ring =
        SBREP_builder::Extract_linear_ring(VE, vx, HE, pm);
    EXPECT_EQ(linear_ring.size(), 9);
    EXPECT_EQ(HE.size(), 0);
  }
}

TEST_F(SBREP_builder_Test, DISABLED_extract_linear_ring_works_with_hole) {
  Polygon_mesh pm;
  Face_group_map fgm;
  size_type num_groups;
  //  Regions planes;
  Regions fgm_regs;

  IO::read_OBJ(file_path, pm, fgm, num_groups);
  const auto &[planes, _] = SBREP_builder::Group_triangles_by_plane(pm);
  fgm_regs = SBREP_builder::Group_triangles(pm, planes, fgm, num_groups);

  auto [HE, VE, vx] = SBREP_builder::Initialize_ds(pm, fgm_regs[0]);
  //  auto he = VE[vx];
  EXPECT_EQ(HE.size(), 16);
  auto linear_ring = SBREP_builder::Extract_linear_ring(VE, vx, HE, pm);
  EXPECT_EQ(linear_ring.size(), 9);
  EXPECT_EQ(HE.size(), 8);
}

TEST_F(SBREP_builder_Test, DISABLED_extract_holes_works_with_hole) {
  Polygon_mesh pm;
  Face_group_map fgm;
  size_type num_groups;
  //  Regions planes;
  Regions fgm_regs;

  IO::read_OBJ(file_path, pm, fgm, num_groups);
  const auto &[planes, _] = SBREP_builder::Group_triangles_by_plane(pm);
  fgm_regs = SBREP_builder::Group_triangles(pm, planes, fgm, num_groups);

  auto [HE, VE, vx] = SBREP_builder::Initialize_ds(pm, fgm_regs[0]);
  //  auto he = VE[vx];
  EXPECT_EQ(HE.size(), 16);
  auto linear_ring = SBREP_builder::Extract_linear_ring(VE, vx, HE, pm);
  EXPECT_EQ(linear_ring.size(), 9);
  EXPECT_EQ(HE.size(), 8);
  auto holes = SBREP_builder::Extract_holes(VE, HE, pm);
  EXPECT_EQ(holes.size(), 1);
  EXPECT_EQ(holes[0].size(), 9);
}

TEST_F(SBREP_builder_Test, DISABLED_extract_holes_works_without_hole) {
  Polygon_mesh pm;
  Face_group_map fgm;
  size_type num_groups;
  //  Regions planes;
  Regions fgm_regs;

  IO::read_OBJ(file_path, pm, fgm, num_groups);
  const auto &[planes, _] = SBREP_builder::Group_triangles_by_plane(pm);
  fgm_regs = SBREP_builder::Group_triangles(pm, planes, fgm, num_groups);

  for (auto i = 1; i < fgm_regs.size(); i++) {
    auto [HE, VE, vx] = SBREP_builder::Initialize_ds(pm, fgm_regs[i]);
    //    auto he = VE[vx];
    EXPECT_EQ(HE.size(), 8);
    auto linear_ring =
        SBREP_builder::Extract_linear_ring(VE, vx, HE, pm);
    EXPECT_EQ(linear_ring.size(), 9);
    EXPECT_EQ(HE.size(), 0);
    auto holes = SBREP_builder::Extract_holes(VE, HE, pm);
    EXPECT_EQ(holes.size(), 0);
  }
}
} // namespace
} // namespace SBREP

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
