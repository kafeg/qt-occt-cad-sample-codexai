#include <gtest/gtest.h>

#include <BoxFeature.h>

#include <TopExp_Explorer.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>

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

TEST(Model, BoxFeatureIsSolidWithBBoxAndVolume) {
  const double dx = 10.0, dy = 20.0, dz = 30.0;
  Handle(BoxFeature) bf = new BoxFeature(dx, dy, dz);
  bf->execute();

  const auto& shp = bf->shape();

  // Basic sanity: produced a solid
  EXPECT_EQ(shp.ShapeType(), TopAbs_SOLID);

  // Bounding box should match dimensions (within small tolerance)
  Bnd_Box bb;
  BRepBndLib::Add(shp, bb);
  Standard_Real xmin = 0, ymin = 0, zmin = 0, xmax = 0, ymax = 0, zmax = 0;
  bb.Get(xmin, ymin, zmin, xmax, ymax, zmax);
  const double tol = 1e-9;
  EXPECT_NEAR(xmax - xmin, dx, tol);
  EXPECT_NEAR(ymax - ymin, dy, tol);
  EXPECT_NEAR(zmax - zmin, dz, tol);

  // Volume check
  GProp_GProps props;
  BRepGProp::VolumeProperties(shp, props);
  const double volume = props.Mass();
  EXPECT_NEAR(volume, dx * dy * dz, 1e-6);
}

