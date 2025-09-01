#include <gtest/gtest.h>

#include <OcctQOpenGLWidgetViewer.h>

#include <QApplication>

// Verify that resetting the view centers at origin and sets fixed eye distance
TEST(Viewer, ResetViewToOriginSetsAtAndEyeDistance)
{
  if (QCoreApplication::instance() == nullptr)
  {
    static int argc = 0;
    static QApplication app(argc, nullptr);
    (void)app;
  }

  OcctQOpenGLWidgetViewer viewer;

  const double kDist = 7.0;
  viewer.resetViewToOrigin(kDist);

  double ex1=0, ey1=0, ez1=0;
  double ax1=0, ay1=0, az1=0;
  viewer.View()->Eye(ex1, ey1, ez1);
  viewer.View()->At(ax1, ay1, az1);

  EXPECT_NEAR(ax1, 0.0, 1.0e-9);
  EXPECT_NEAR(ay1, 0.0, 1.0e-9);
  EXPECT_NEAR(az1, 0.0, 1.0e-9);

  EXPECT_NEAR(ex1, kDist, 1.0e-9);
  EXPECT_NEAR(ey1, -kDist, 1.0e-9);
  EXPECT_NEAR(ez1, kDist, 1.0e-9);
}
