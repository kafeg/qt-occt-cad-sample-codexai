#include <gtest/gtest.h>

#include <CylinderFeature.h>

#include <TopExp_Explorer.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopoDS.hxx>
#include <cmath>
#include <common/test_utils.h>

TEST(Model, CylinderFeatureProducesThreeFaces) {
  Handle(CylinderFeature) cf = new CylinderFeature();
  cf->set(10.0, 20.0);
  cf->execute();

  const auto& shp = cf->shape();
  EXPECT_EQ(countFaces(shp), 3);
}

TEST(Model, CylinderFeatureFaceTypesBBoxAndVolume) {
  const double r = 10.0;
  const double h = 20.0;
  Handle(CylinderFeature) cf = new CylinderFeature(r, h);
  cf->execute();

  const auto& shp = cf->shape();

  // Solid check
  EXPECT_EQ(shp.ShapeType(), TopAbs_SOLID);

  // Face type distribution: 2 planar caps + 1 cylindrical lateral
  auto st = countSurfaceTypes(shp);
  EXPECT_EQ(st.planar, 2);
  EXPECT_EQ(st.cylindrical, 1);

  // Bounding box: 2r x 2r x h (axis is Z)
  auto ext = bboxExtents(shp);
  const double tol = 1e-6;
  EXPECT_NEAR(ext[0], 2.0 * r, tol);
  EXPECT_NEAR(ext[1], 2.0 * r, tol);
  EXPECT_NEAR(ext[2], h, tol);

  // Volume: pi * r^2 * h
  const double expected = std::acos(-1.0) * r * r * h;
  EXPECT_NEAR(volume(shp), expected, 1e-4);
}
