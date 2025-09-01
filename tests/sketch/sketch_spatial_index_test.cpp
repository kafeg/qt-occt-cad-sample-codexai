#include <gtest/gtest.h>

#include "Sketch.h"

#include <chrono>
#include <random>

TEST(SketchSpatialIndexTest, NearCoincidentEndpointsAutoWeld)
{
  Sketch s;
  // Two lines whose shared endpoints are within tolerance, without explicit constraint
  auto l0 = s.addLine(gp_Pnt2d(0.0, 0.0), gp_Pnt2d(10.0, 0.0));
  // l1 starts near l0 end (within tol)
  double eps = 1e-7; // tolerance used in solve below
  auto l1 = s.addLine(gp_Pnt2d(10.0 + eps * 0.5, 0.0), gp_Pnt2d(20.0, 0.0));

  s.solveConstraints(eps);

  // Endpoints should have been welded by geometric proximity
  auto pA = s.curves()[l0].line.p2;
  auto pB = s.curves()[l1].line.p1;
  EXPECT_DOUBLE_EQ(pA.X(), pB.X());
  EXPECT_DOUBLE_EQ(pA.Y(), pB.Y());

  const auto wires = s.computeWires(eps);
  ASSERT_EQ(wires.size(), 1u);
  ASSERT_EQ(wires[0].curves.size(), 2u);
}

TEST(SketchSpatialIndexTest, PerformanceOnLargeInput)
{
  Sketch s;

  // Deterministic random generator
  std::mt19937 rng(12345);
  std::uniform_real_distribution<double> dist(-10000.0, 10000.0);

  // Create many disjoint lines so there are no near-coincidences
  const int N = 3000; // 3000 lines -> 6000 endpoints
  for (int i = 0; i < N; ++i)
  {
    double x1 = dist(rng);
    double y1 = dist(rng);
    double x2 = x1 + 1.0; // short segment offset
    double y2 = y1 + 0.5;
    s.addLine(gp_Pnt2d(x1, y1), gp_Pnt2d(x2, y2));
  }

  auto t0 = std::chrono::steady_clock::now();
  s.solveConstraints(1e-9);
  auto t1 = std::chrono::steady_clock::now();
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

  // Expect solve to be reasonably fast with spatial index
  // Use a generous threshold to avoid flakiness on CI
  EXPECT_LT(ms, 2000) << "solveConstraints took too long: " << ms << " ms";
}

