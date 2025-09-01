#include <gtest/gtest.h>

#include <CylinderFeature.h>

#include <TopExp_Explorer.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopoDS.hxx>
#include <cmath>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>
#include <BRep_Tool.hxx>
#include <GeomAdaptor_Surface.hxx>
#include <GeomAbs_SurfaceType.hxx>

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

TEST(Model, CylinderFeatureFaceTypesBBoxAndVolume) {
  const double r = 10.0;
  const double h = 20.0;
  Handle(CylinderFeature) cf = new CylinderFeature(r, h);
  cf->execute();

  const auto& shp = cf->shape();

  // Solid check
  EXPECT_EQ(shp.ShapeType(), TopAbs_SOLID);

  // Face type distribution: 2 planar caps + 1 cylindrical lateral
  int planarCount = 0;
  int cylindricalCount = 0;
  for (TopExp_Explorer exp(shp, TopAbs_FACE); exp.More(); exp.Next()) {
    const TopoDS_Face& f = TopoDS::Face(exp.Current());
    Handle(Geom_Surface) s = BRep_Tool::Surface(f);
    GeomAdaptor_Surface ga(s);
    switch (ga.GetType()) {
      case GeomAbs_Plane:
        ++planarCount;
        break;
      case GeomAbs_Cylinder:
        ++cylindricalCount;
        break;
      default:
        break;
    }
  }
  EXPECT_EQ(planarCount, 2);
  EXPECT_EQ(cylindricalCount, 1);

  // Bounding box: 2r x 2r x h (axis is Z)
  Bnd_Box bb;
  BRepBndLib::Add(shp, bb);
  Standard_Real xmin = 0, ymin = 0, zmin = 0, xmax = 0, ymax = 0, zmax = 0;
  bb.Get(xmin, ymin, zmin, xmax, ymax, zmax);
  const double tol = 1e-6;
  EXPECT_NEAR(xmax - xmin, 2.0 * r, tol);
  EXPECT_NEAR(ymax - ymin, 2.0 * r, tol);
  EXPECT_NEAR(zmax - zmin, h, tol);

  // Volume: pi * r^2 * h
  GProp_GProps props;
  BRepGProp::VolumeProperties(shp, props);
  const double volume = props.Mass();
  const double expected = std::acos(-1.0) * r * r * h;
  EXPECT_NEAR(volume, expected, 1e-4);
}

