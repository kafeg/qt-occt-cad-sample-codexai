// Minimal kernel API wrapper over OCCT primitives/booleans (no Qt here)
#pragma once

#include <TopoDS_Shape.hxx>

namespace KernelAPI
{
  // Create a box primitive
  TopoDS_Shape makeBox(double dx, double dy, double dz);

  // Boolean fuse (union)
  TopoDS_Shape fuse(const TopoDS_Shape& a, const TopoDS_Shape& b);
}

