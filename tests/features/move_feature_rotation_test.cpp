#include <gtest/gtest.h>

#include <BoxFeature.h>
#include <MoveFeature.h>

#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <Bnd_Box.hxx>
#include <gp_Quaternion.hxx>
#include <gp_EulerSequence.hxx>

static gp_Pnt bboxCenter(const TopoDS_Shape& s)
{
  Bnd_Box bb; BRepBndLib::Add(s, bb);
  Standard_Real xmin=0, ymin=0, zmin=0, xmax=0, ymax=0, zmax=0;
  bb.Get(xmin, ymin, zmin, xmax, ymax, zmax);
  return gp_Pnt(0.5*(xmin+xmax), 0.5*(ymin+ymax), 0.5*(zmin+zmax));
}

static gp_Trsf rotateAroundPointXYZ(const gp_Pnt& p, double rxDeg, double ryDeg, double rzDeg)
{
  const double rx = rxDeg * (M_PI / 180.0);
  const double ry = ryDeg * (M_PI / 180.0);
  const double rz = rzDeg * (M_PI / 180.0);
  gp_Quaternion q; q.SetEulerAngles(gp_Intrinsic_XYZ, rx, ry, rz);
  // Translation that fixes point p: t = p - R * p
  gp_Mat R = q.GetMatrix();
  gp_XYZ px = p.XYZ();
  gp_XYZ Rp = px; Rp.Multiply(R);
  gp_Trsf tr; tr.SetTransformation(q, gp_Vec(px.X() - Rp.X(), px.Y() - Rp.Y(), px.Z() - Rp.Z()));
  return tr;
}

TEST(MoveFeature, CombinedRotationKeepsCenterWithExactDelta)
{
  Handle(BoxFeature) src = new BoxFeature(10.0, 20.0, 30.0);
  src->execute();
  const gp_Pnt c0 = bboxCenter(src->shape());

  // Build combined rotation around the shape center (no net translation of the center)
  const gp_Trsf delta = rotateAroundPointXYZ(c0, 30.0, 25.0, 10.0);

  Handle(MoveFeature) mf = new MoveFeature();
  mf->setSource(src);
  mf->setSourceId(src->id());
  mf->setDeltaTrsf(delta); // use exact affine delta
  // Optional: store decomposed params for UI; should not affect execution
  gp_Quaternion q = delta.GetRotation();
  Standard_Real ax=0, ay=0, az=0; q.GetEulerAngles(gp_Intrinsic_XYZ, ax, ay, az);
  const double r2d = 180.0 / M_PI;
  mf->setRotation(ax*r2d, ay*r2d, az*r2d);
  gp_XYZ t = delta.TranslationPart();
  mf->setTranslation(t.X(), t.Y(), t.Z());

  mf->execute();
  const gp_Pnt c1 = bboxCenter(mf->shape());

  // The rotation is around the center -> center must be preserved
  EXPECT_NEAR(c0.X(), c1.X(), 1.0e-7);
  EXPECT_NEAR(c0.Y(), c1.Y(), 1.0e-7);
  EXPECT_NEAR(c0.Z(), c1.Z(), 1.0e-7);

  // And the result should match direct transform by delta
  TopoDS_Shape expected = BRepBuilderAPI_Transform(src->shape(), delta, true).Shape();
  const gp_Pnt cExp = bboxCenter(expected);
  EXPECT_NEAR(cExp.X(), c1.X(), 1.0e-7);
  EXPECT_NEAR(cExp.Y(), c1.Y(), 1.0e-7);
  EXPECT_NEAR(cExp.Z(), c1.Z(), 1.0e-7);
}

TEST(MoveFeature, FallbackEulerMatchesExactDelta)
{
  Handle(BoxFeature) src = new BoxFeature(12.0, 18.0, 9.0);
  src->execute();
  const gp_Pnt c0 = bboxCenter(src->shape());

  const gp_Trsf delta = rotateAroundPointXYZ(c0, 40.0, 15.0, -25.0);
  const gp_Quaternion q = delta.GetRotation();
  gp_XYZ t = delta.TranslationPart();
  Standard_Real ax=0, ay=0, az=0; q.GetEulerAngles(gp_Intrinsic_XYZ, ax, ay, az);

  Handle(MoveFeature) mf = new MoveFeature();
  mf->setSource(src);
  mf->setSourceId(src->id());
  // Do NOT set delta to force Euler fallback path
  mf->setTranslation(t.X(), t.Y(), t.Z());
  mf->setRotation(ax * 180.0 / M_PI, ay * 180.0 / M_PI, az * 180.0 / M_PI);

  mf->execute();

  TopoDS_Shape expected = BRepBuilderAPI_Transform(src->shape(), delta, true).Shape();
  const gp_Pnt c1 = bboxCenter(mf->shape());
  const gp_Pnt cExp = bboxCenter(expected);
  EXPECT_NEAR(cExp.X(), c1.X(), 1.0e-7);
  EXPECT_NEAR(cExp.Y(), c1.Y(), 1.0e-7);
  EXPECT_NEAR(cExp.Z(), c1.Z(), 1.0e-7);
}

TEST(MoveFeature, SequentialMovesCompose)
{
  Handle(BoxFeature) src = new BoxFeature(10.0, 10.0, 10.0);
  src->execute();
  const gp_Pnt c0 = bboxCenter(src->shape());

  gp_Trsf d1 = rotateAroundPointXYZ(c0, 10.0, 20.0, 30.0);
  gp_Trsf d2 = rotateAroundPointXYZ(c0, -15.0, 5.0, 45.0);

  Handle(MoveFeature) m1 = new MoveFeature();
  m1->setSource(src);
  m1->setSourceId(src->id());
  m1->setDeltaTrsf(d1);
  m1->execute();

  Handle(MoveFeature) m2 = new MoveFeature();
  m2->setSource(m1);
  m2->setSourceId(m1->id());
  m2->setDeltaTrsf(d2);
  m2->execute();

  gp_Trsf d12 = d1 * d2;
  TopoDS_Shape expected = BRepBuilderAPI_Transform(src->shape(), d12, true).Shape();

  const gp_Pnt cRes = bboxCenter(m2->shape());
  const gp_Pnt cExp = bboxCenter(expected);
  EXPECT_NEAR(cExp.X(), cRes.X(), 1.0e-7);
  EXPECT_NEAR(cExp.Y(), cRes.Y(), 1.0e-7);
  EXPECT_NEAR(cExp.Z(), cRes.Z(), 1.0e-7);
}

