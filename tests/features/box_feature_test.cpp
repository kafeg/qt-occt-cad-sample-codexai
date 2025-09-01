#include <gtest/gtest.h>

#include <BoxFeature.h>

#include <TopExp_Explorer.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <common/test_utils.h>

TEST(Model, BoxFeatureProducesSixFaces) {
  Handle(BoxFeature) bf = new BoxFeature();
  bf->setSize(10.0, 20.0, 30.0);
  bf->execute();

  const auto& shp = bf->shape();
  EXPECT_EQ(countFaces(shp), 6);
}

TEST(Model, BoxFeatureIsSolidWithBBoxAndVolume) {
  const double dx = 10.0, dy = 20.0, dz = 30.0;
  Handle(BoxFeature) bf = new BoxFeature(dx, dy, dz);
  bf->execute();

  const auto& shp = bf->shape();

  // Basic sanity: produced a solid
  EXPECT_EQ(shp.ShapeType(), TopAbs_SOLID);

  // Bounding box should match dimensions (within small tolerance)
  auto ext = bboxExtents(shp);
  const double tol = 1e-6;
  EXPECT_NEAR(ext[0], dx, tol);
  EXPECT_NEAR(ext[1], dy, tol);
  EXPECT_NEAR(ext[2], dz, tol);

  // Volume check
  EXPECT_NEAR(volume(shp), dx * dy * dz, 1e-6);
}
