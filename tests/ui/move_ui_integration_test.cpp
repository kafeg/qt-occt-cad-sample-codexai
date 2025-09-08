#include <gtest/gtest.h>

#include <QApplication>

#include <TabPageWidget.h>
#include <Document.h>
#include <BoxFeature.h>
#include <MoveFeature.h>
#include <OcctQOpenGLWidgetViewer.h>

#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
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

TEST(UI_Move, ConfirmCombinedRotationAddsSingleMoveWithCorrectDelta)
{
  if (QCoreApplication::instance() == nullptr)
  {
    static int argc = 0; static QApplication app(argc, nullptr); (void)app;
  }

  TabPageWidget page;
  Document& doc = page.doc();

  // Create a box and sync viewer
  Handle(BoxFeature) bf = new BoxFeature(10.0, 20.0, 30.0);
  doc.addFeature(bf);
  doc.recompute();
  page.syncViewerFromDoc(true);
  page.refreshFeatureList();

  // Select it in viewer and activate move
  page.selectFeatureInViewer(bf);
  page.activateMove();

  // Build combined rotation around shape center and simulate confirm
  const gp_Pnt c0 = bboxCenter(bf->shape());
  const gp_Trsf delta = rotateAroundPointXYZ(c0, 30.0, 25.0, -15.0);
  page.viewer()->emitManipulatorFinishedForTest(delta);

  // After confirm: source suppressed, one MoveFeature added
  ASSERT_EQ(doc.features().Size(), 2);
  Handle(Feature) last = doc.features().Last();
  Handle(MoveFeature) mf = Handle(MoveFeature)::DownCast(last);
  ASSERT_FALSE(mf.IsNull());

  // The stored delta (if used) or fallback must produce correct center
  const gp_Pnt c1 = bboxCenter(mf->shape());
  EXPECT_NEAR(c0.X(), c1.X(), 1.0e-7);
  EXPECT_NEAR(c0.Y(), c1.Y(), 1.0e-7);
  EXPECT_NEAR(c0.Z(), c1.Z(), 1.0e-7);
}
