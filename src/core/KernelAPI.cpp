#include "KernelAPI.h"

#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepAlgoAPI_Fuse.hxx>

namespace KernelAPI
{
TopoDS_Shape makeBox(double dx, double dy, double dz)
{
  return BRepPrimAPI_MakeBox(dx, dy, dz).Shape();
}

TopoDS_Shape fuse(const TopoDS_Shape& a, const TopoDS_Shape& b)
{
  return BRepAlgoAPI_Fuse(a, b).Shape();
}
}

