#include <gtest/gtest.h>

#include <BoxFeature.h>

#include <TopExp_Explorer.hxx>
#include <TopAbs_ShapeEnum.hxx>

TEST(Model, BoxFeatureProducesSixFaces) {
  Handle(BoxFeature) bf = new BoxFeature();
  bf->setSize(10.0, 20.0, 30.0);
  bf->execute();

  const auto& shp = bf->shape();
  int faceCount = 0;
  for (TopExp_Explorer exp(shp, TopAbs_FACE); exp.More(); exp.Next()) {
    ++faceCount;
  }
  EXPECT_EQ(faceCount, 6);
}
