#include "KernelAPI.h"

#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <TopExp_Explorer.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <gp_Vec.hxx>

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

// Extrude a set of wires along +Z by a given distance (compat wrapper)
TopoDS_Shape extrude(const std::vector<TopoDS_Wire>& wires, double distance)
{
  return extrude(wires, gp_Vec(0.0, 0.0, distance));
}

// Extrude a set of wires along arbitrary vector direction
TopoDS_Shape extrude(const std::vector<TopoDS_Wire>& wires, const gp_Vec& dir)
{
  if (wires.empty() || dir.SquareMagnitude() <= gp::Resolution())
  {
    return TopoDS_Shape();
  }

  TopoDS_Shape result;

  for (const TopoDS_Wire& w : wires)
  {
    if (w.IsNull()) { continue; }
    TopoDS_Face face = BRepBuilderAPI_MakeFace(w);
    if (face.IsNull()) { continue; }
    TopoDS_Shape prism = BRepPrimAPI_MakePrism(face, dir).Shape();
    if (result.IsNull())
    {
      result = prism;
    }
    else
    {
      result = BRepAlgoAPI_Fuse(result, prism).Shape();
    }
  }
  return result;
}
}
