#include "KernelAPI.h"

#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepAlgoAPI_Fuse.hxx>

namespace KernelAPI
{
// Box: OCCT builder returns a closed solid with 6 planar faces
TopoDS_Shape makeBox(double dx, double dy, double dz)
{
  return BRepPrimAPI_MakeBox(dx, dy, dz).Shape();
}

// Cylinder: oriented along +Z (two caps + lateral cylindrical face)
TopoDS_Shape makeCylinder(double radius, double height)
{
  return BRepPrimAPI_MakeCylinder(radius, height).Shape();
}

// Fuse: unified solid (may produce shells if inputs are not solids)
TopoDS_Shape fuse(const TopoDS_Shape& a, const TopoDS_Shape& b)
{
  return BRepAlgoAPI_Fuse(a, b).Shape();
}
}
