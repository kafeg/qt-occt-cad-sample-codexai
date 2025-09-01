#include <gtest/gtest.h>

#include <OcctQOpenGLWidgetViewer.h>

#include <QApplication>

// Verify that resetting the view centers at origin and scales eye distance
TEST(Viewer, ResetViewToOriginSetsAtAndScalesEyeDistance)
{
  if (QCoreApplication::instance() == nullptr)
  {
    static int argc = 0;
    static QApplication app(argc, nullptr);
    (void)app;
  }

  OcctQOpenGLWidgetViewer viewer;

  double ex0=0, ey0=0, ez0=0;
  double ax0=0, ay0=0, az0=0;
  viewer.View()->Eye(ex0, ey0, ez0);
  viewer.View()->At(ax0, ay0, az0);
  const double d0 = gp_Pnt(ex0, ey0, ez0).Distance(gp_Pnt(ax0, ay0, az0));

  viewer.resetViewToOrigin();

  double ex1=0, ey1=0, ez1=0;
  double ax1=0, ay1=0, az1=0;
  viewer.View()->Eye(ex1, ey1, ez1);
  viewer.View()->At(ax1, ay1, az1);

  EXPECT_NEAR(ax1, 0.0, 1.0e-9);
  EXPECT_NEAR(ay1, 0.0, 1.0e-9);
  EXPECT_NEAR(az1, 0.0, 1.0e-9);

  const double d1 = gp_Pnt(ex1, ey1, ez1).Distance(gp_Pnt(ax1, ay1, az1));
  // Allow a small relative tolerance
  EXPECT_NEAR(d1, 0.20 * d0, d0 * 0.05 + 1.0e-9);
}
