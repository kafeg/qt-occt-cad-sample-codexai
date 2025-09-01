// Minimal kernel API: thin wrappers over OCCT BRepPrimAPI/BRepAlgoAPI (no Qt deps)
#pragma once

#include <TopoDS_Shape.hxx>
#include <TopoDS_Wire.hxx>
#include <vector>

namespace KernelAPI
{
  // Create a box primitive with edges aligned to XYZ axes
  TopoDS_Shape makeBox(double dx, double dy, double dz);
  // Create a right circular cylinder along +Z with given radius and height
  TopoDS_Shape makeCylinder(double radius, double height);

  // Boolean fuse (union) of two shapes; returns the combined solid
  TopoDS_Shape fuse(const TopoDS_Shape& a, const TopoDS_Shape& b);

  // Linear extrusion (prism) of one or more planar profile wires along +Z by a distance
  // - Each wire is treated independently and the resulting prisms are fused
  // - Input wires are assumed to lie in the XY plane (Z=0)
  TopoDS_Shape extrude(const std::vector<TopoDS_Wire>& wires, double distance);
}
