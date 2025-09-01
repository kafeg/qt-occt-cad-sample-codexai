#include <gtest/gtest.h>

#include <CylinderFeature.h>

#include <TopExp_Explorer.hxx>
#include <TopAbs_ShapeEnum.hxx>

TEST(Model, CylinderFeatureProducesThreeFaces) {
  Handle(CylinderFeature) cf = new CylinderFeature();
  cf->set(10.0, 20.0);
  cf->execute();

  const auto& shp = cf->shape();
  int faceCount = 0;
  for (TopExp_Explorer exp(shp, TopAbs_FACE); exp.More(); exp.Next()) {
    ++faceCount;
  }
  EXPECT_EQ(faceCount, 3);
}

