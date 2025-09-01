#include <gtest/gtest.h>

#include "Sketch.h"

TEST(SketchStorageTest, AddLinesAndArcs)
{
  Sketch s;

  auto l0 = s.addLine(gp_Pnt2d(0, 0), gp_Pnt2d(10, 0));
  auto l1 = s.addLine(gp_Pnt2d(10, 0), gp_Pnt2d(10, 10));
  auto a0 = s.addArc(gp_Pnt2d(5, 5), gp_Pnt2d(6, 5), gp_Pnt2d(5, 6), true);

  ASSERT_EQ(l0, 0);
  ASSERT_EQ(l1, 1);
  ASSERT_EQ(a0, 2);

  const auto& curves = s.curves();
  ASSERT_EQ(curves.size(), 3u);

  ASSERT_EQ(curves[0].type, Sketch::CurveType::Line);
  EXPECT_DOUBLE_EQ(curves[0].line.p1.X(), 0.0);
  EXPECT_DOUBLE_EQ(curves[0].line.p1.Y(), 0.0);
  EXPECT_DOUBLE_EQ(curves[0].line.p2.X(), 10.0);
  EXPECT_DOUBLE_EQ(curves[0].line.p2.Y(), 0.0);

  ASSERT_EQ(curves[2].type, Sketch::CurveType::Arc);
  EXPECT_DOUBLE_EQ(curves[2].arc.center.X(), 5.0);
  EXPECT_DOUBLE_EQ(curves[2].arc.center.Y(), 5.0);
  EXPECT_TRUE(curves[2].arc.clockwise);
}
