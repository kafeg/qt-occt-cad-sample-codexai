#include <gtest/gtest.h>

#include "Sketch.h"

namespace
{
bool containsAll(const std::vector<int>& a, const std::vector<int>& expect)
{
  for (int v : expect)
  {
    if (std::find(a.begin(), a.end(), v) == a.end())
      return false;
  }
  return true;
}
}  // namespace

TEST(SketchConstraintsTest, CoincidentConnectsLinesIntoSingleWire)
{
  Sketch s;
  // Two lines end-to-end but with tiny gap
  auto l0 = s.addLine(gp_Pnt2d(0, 0), gp_Pnt2d(10, 0));
  auto l1 = s.addLine(gp_Pnt2d(10 + 1e-6, 0), gp_Pnt2d(20, 0));

  // Constrain coincident between l0 end and l1 start
  s.addCoincident(Sketch::EndpointRef{l0, 1}, Sketch::EndpointRef{l1, 0});
  s.solveConstraints();

  // Endpoints should now be exactly equal
  auto pA = s.curves()[l0].line.p2;
  auto pB = s.curves()[l1].line.p1;
  EXPECT_DOUBLE_EQ(pA.X(), pB.X());
  EXPECT_DOUBLE_EQ(pA.Y(), pB.Y());

  const auto wires = s.computeWires();
  ASSERT_EQ(wires.size(), 1u);
  ASSERT_EQ(wires[0].curves.size(), 2u);
  EXPECT_TRUE(containsAll(wires[0].curves, {l0, l1}));
}

TEST(SketchConstraintsTest, ThreeSegmentChainFormsOneWire)
{
  Sketch s;
  auto l0 = s.addLine(gp_Pnt2d(0, 0), gp_Pnt2d(10, 0));
  auto l1 = s.addLine(gp_Pnt2d(10, 0), gp_Pnt2d(10, 10));
  auto l2 = s.addLine(gp_Pnt2d(10, 10), gp_Pnt2d(20, 10));

  // Explicit coincident constraints
  s.addCoincident(Sketch::EndpointRef{l0, 1}, Sketch::EndpointRef{l1, 0});
  s.addCoincident(Sketch::EndpointRef{l1, 1}, Sketch::EndpointRef{l2, 0});
  s.solveConstraints();

  const auto wires = s.computeWires();
  ASSERT_EQ(wires.size(), 1u);
  ASSERT_EQ(wires[0].curves.size(), 3u);
  EXPECT_TRUE(containsAll(wires[0].curves, {l0, l1, l2}));
}

TEST(SketchConstraintsTest, DisjointCreatesTwoWires)
{
  Sketch s;
  auto l0 = s.addLine(gp_Pnt2d(0, 0), gp_Pnt2d(10, 0));
  auto l1 = s.addLine(gp_Pnt2d(100, 0), gp_Pnt2d(110, 0));
  s.solveConstraints();

  const auto wires = s.computeWires();
  ASSERT_EQ(wires.size(), 2u);
}
