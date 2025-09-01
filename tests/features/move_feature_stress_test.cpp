#include <gtest/gtest.h>

#include <Document.h>
#include <BoxFeature.h>
#include <MoveFeature.h>

#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_Transform.hxx>
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
  gp_Mat R = q.GetMatrix();
  gp_XYZ px = p.XYZ();
  gp_XYZ Rp = px; Rp.Multiply(R);
  gp_Trsf tr; tr.SetTransformation(q, gp_Vec(px.X() - Rp.X(), px.Y() - Rp.Y(), px.Z() - Rp.Z()));
  return tr;
}

TEST(MoveFeature, Stress_ChainOfManyMoves)
{
  Document doc;
  Handle(BoxFeature) base = new BoxFeature(10.0, 12.0, 8.0);
  doc.addFeature(base);
  doc.recompute();

  const TopoDS_Shape baseShape = base->shape();
  const gp_Pnt baseCenter = bboxCenter(baseShape);

  const int N = 140; // 100+ mixed transforms
  gp_Trsf accum; // identity

  Handle(Feature) prev = base;
  for (int i = 0; i < N; ++i)
  {
    // Deterministic varying angles/translations
    const double rx = (i % 7 == 0) ? 0.0 : (5.0 + (i % 13));
    const double ry = (i % 5 == 0) ? 0.0 : (3.0 + (i % 11));
    const double rz = (i % 3 == 0) ? 0.0 : (2.0 + (i % 9));
    const gp_Vec t( (i % 2 == 0 ? 0.2 * (i % 4) : -0.1 * (i % 3)),
                    (i % 3 == 0 ? 0.15 * (i % 5) : -0.05 * (i % 7)),
                    (i % 4 == 0 ? 0.1 * (i % 6) : -0.07 * (i % 5)) );

    // Current expected center under accum
    const gp_Pnt currCenter = bboxCenter(BRepBuilderAPI_Transform(baseShape, accum, true).Shape());
    gp_Trsf dRot = rotateAroundPointXYZ(currCenter, rx, ry, rz);
    gp_Trsf dTrans; dTrans.SetTranslation(t);
    // Apply translation after rotation (world space move after rotating body around its center)
    gp_Trsf delta = dTrans * dRot;

    // Expected accumulate matches viewer rule (pre-multiply)
    accum = delta * accum;

    Handle(MoveFeature) mf = new MoveFeature();
    mf->setSourceId(prev->id());
    mf->setDeltaTrsf(delta);
    // for readability in history
    gp_Quaternion q = delta.GetRotation();
    Standard_Real ax=0, ay=0, az=0; q.GetEulerAngles(gp_Intrinsic_XYZ, ax, ay, az);
    const double r2d = 180.0 / M_PI;
    mf->setRotation(ax*r2d, ay*r2d, az*r2d);
    gp_XYZ tt = delta.TranslationPart();
    mf->setTranslation(tt.X(), tt.Y(), tt.Z());

    // Match UI behavior: hide previous body
    prev->setSuppressed(true);
    doc.addFeature(mf);
    doc.recompute();
    prev = mf;
  }

  ASSERT_EQ(doc.features().Size(), N + 1);
  Handle(Feature) last = doc.features().Last();
  Handle(MoveFeature) lastMove = Handle(MoveFeature)::DownCast(last);
  ASSERT_FALSE(lastMove.IsNull());

  // Compare final centers
  const gp_Pnt cExpected = bboxCenter(BRepBuilderAPI_Transform(baseShape, accum, true).Shape());
  const gp_Pnt cActual   = bboxCenter(lastMove->shape());
  EXPECT_NEAR(cExpected.X(), cActual.X(), 1.0e-6);
  EXPECT_NEAR(cExpected.Y(), cActual.Y(), 1.0e-6);
  EXPECT_NEAR(cExpected.Z(), cActual.Z(), 1.0e-6);
}

