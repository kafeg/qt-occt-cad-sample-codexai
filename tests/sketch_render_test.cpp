#include <gtest/gtest.h>

#include <OcctQOpenGLWidgetViewer.h>
#include <Sketch.h>

#include <QApplication>

// Basic render-path coverage: ensure a sketch can be added to the viewer
TEST(ViewerSketch, AddSingleLineSketch)
{
  static int argc = 0;
  static QApplication app(argc, nullptr);

  OcctQOpenGLWidgetViewer viewer;
  auto sk = std::make_shared<Sketch>();
  sk->addLine(gp_Pnt2d(0.0, 0.0), gp_Pnt2d(10.0, 0.0));

  auto h = viewer.addSketch(sk);
  EXPECT_FALSE(h.IsNull());
  EXPECT_GT(viewer.sketchCount(), 0);
}

