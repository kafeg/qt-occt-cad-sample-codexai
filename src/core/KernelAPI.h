// Minimal kernel API: thin wrappers over OCCT BRepPrimAPI/BRepAlgoAPI (no Qt deps)
#pragma once

#include <TopoDS_Shape.hxx>

namespace KernelAPI
{
  // Create a box primitive with edges aligned to XYZ axes
  TopoDS_Shape makeBox(double dx, double dy, double dz);
  // Create a right circular cylinder along +Z with given radius and height
  TopoDS_Shape makeCylinder(double radius, double height);

  // Boolean fuse (union) of two shapes; returns the combined solid
  TopoDS_Shape fuse(const TopoDS_Shape& a, const TopoDS_Shape& b);
}
