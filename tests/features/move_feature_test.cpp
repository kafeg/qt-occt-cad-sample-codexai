#include <gtest/gtest.h>

#include <Document.h>
#include <BoxFeature.h>
#include <MoveFeature.h>

#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>

static gp_Pnt bboxCenter(const TopoDS_Shape& s)
{
  Bnd_Box bb; BRepBndLib::Add(s, bb);
  Standard_Real xmin = 0, ymin = 0, zmin = 0, xmax = 0, ymax = 0, zmax = 0;
  bb.Get(xmin, ymin, zmin, xmax, ymax, zmax);
  return gp_Pnt(0.5*(xmin+xmax), 0.5*(ymin+ymax), 0.5*(zmin+zmax));
}

TEST(Model, MoveFeatureTranslatesShape)
{
  Document doc;
  Handle(BoxFeature) bf = new BoxFeature(10.0, 20.0, 30.0);
  doc.addFeature(bf);
  doc.recompute();

  const gp_Pnt c0 = bboxCenter(bf->shape());

  Handle(MoveFeature) mf = new MoveFeature();
  mf->setSourceId(bf->id());
  mf->setTranslation(5.0, -3.0, 2.5);
  mf->setRotation(0.0, 0.0, 0.0);
  doc.addFeature(mf);
  doc.recompute();

  const gp_Pnt c1 = bboxCenter(mf->shape());
  EXPECT_NEAR(c1.X() - c0.X(), 5.0, 1e-7);
  EXPECT_NEAR(c1.Y() - c0.Y(), -3.0, 1e-7);
  EXPECT_NEAR(c1.Z() - c0.Z(), 2.5, 1e-7);
}

