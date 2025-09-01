#include <gtest/gtest.h>

#include "Sketch.h"

#include <TopExp_Explorer.hxx>
#include <TopAbs_ShapeEnum.hxx>

static int edgeCount(const TopoDS_Shape& s)
{
  int n = 0;
  for (TopExp_Explorer ex(s, TopAbs_EDGE); ex.More(); ex.Next())
    ++n;
  return n;
}

TEST(SketchOrderExportTest, ThreeSegmentChainOrdersAndExports)
{
  Sketch s;
  auto a = s.addLine(gp_Pnt2d(0, 0), gp_Pnt2d(10, 0));
  auto b = s.addLine(gp_Pnt2d(10, 0), gp_Pnt2d(10, 10));
  auto c = s.addLine(gp_Pnt2d(10, 10), gp_Pnt2d(20, 10));

  s.addCoincident({a, 1}, {b, 0});
  s.addCoincident({b, 1}, {c, 0});
  s.solveConstraints();

  auto paths = s.computeOrderedPaths();
  ASSERT_EQ(paths.size(), 1u);
  ASSERT_EQ(paths[0].size(), 3u);

  auto wires = s.toOcctWires();
  ASSERT_EQ(wires.size(), 1u);
  EXPECT_EQ(edgeCount(wires[0]), 3);
}

TEST(SketchOrderExportTest, ArcAndLineWireExportsTwoEdges)
{
  Sketch s;
  // Quarter arc from (1,0) to (0,1) centered at origin, then a line
  auto arc = s.addArc(gp_Pnt2d(0, 0), gp_Pnt2d(1, 0), gp_Pnt2d(0, 1), false /*ccw*/);
  auto ln  = s.addLine(gp_Pnt2d(0, 1), gp_Pnt2d(0, 2));
  s.addCoincident({arc, 1}, {ln, 0});
  s.solveConstraints();

  auto wires = s.toOcctWires();
  ASSERT_EQ(wires.size(), 1u);
  EXPECT_EQ(edgeCount(wires[0]), 2);
}

