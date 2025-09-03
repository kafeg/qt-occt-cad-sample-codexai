#include <gtest/gtest.h>

#include <FiniteGrid.h>

// Unit test the adaptive grid step snapping and extent recomputation logic
TEST(FiniteGrid, StepAndExtentsRespondToZoom)
{
  Handle(FiniteGrid) grid = new FiniteGrid();

  // Simulate a viewport and a coarse scale (zoomed out): large world per 20px
  grid->updateFromViewportSample(800, 600, /*worldPer20px*/ 200.0);
  const double stepOut  = grid->step();
  const double widthOut = grid->xmax() - grid->xmin();
  ASSERT_GT(stepOut, 0.0);
  ASSERT_GT(widthOut, 0.0);

  // Simulate zooming in: much smaller world distance per 20px
  grid->updateFromViewportSample(800, 600, /*worldPer20px*/ 10.0);
  const double stepIn  = grid->step();
  const double widthIn = grid->xmax() - grid->xmin();

  EXPECT_LT(stepIn, stepOut) << "Grid step should decrease when zooming in";
  EXPECT_LT(widthIn, widthOut) << "Grid extents should shrink when zooming in";

  // Simulate zooming out further: very large world per 20px
  grid->updateFromViewportSample(800, 600, /*worldPer20px*/ 1000.0);
  const double stepOut2  = grid->step();
  const double widthOut2 = grid->xmax() - grid->xmin();

  EXPECT_GT(stepOut2, stepIn) << "Grid step should increase when zooming out";
  EXPECT_GT(widthOut2, widthIn) << "Grid extents should grow when zooming out";
}

