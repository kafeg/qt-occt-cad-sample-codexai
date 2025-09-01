#pragma once

#include <TopExp_Explorer.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS.hxx>

#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <BRep_Tool.hxx>
#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>

#include <Geom_Surface.hxx>
#include <GeomAdaptor_Surface.hxx>
#include <GeomAbs_SurfaceType.hxx>

#include <array>

// Count faces of a shape
inline int countFaces(const TopoDS_Shape& shape)
{
  int n = 0; for (TopExp_Explorer exp(shape, TopAbs_FACE); exp.More(); exp.Next()) { ++n; } return n;
}

// Compute axis-aligned bounding box extents (dx, dy, dz)
inline std::array<double, 3> bboxExtents(const TopoDS_Shape& shape)
{
  Bnd_Box bb; BRepBndLib::Add(shape, bb);
  Standard_Real xmin = 0, ymin = 0, zmin = 0, xmax = 0, ymax = 0, zmax = 0;
  bb.Get(xmin, ymin, zmin, xmax, ymax, zmax);
  return { xmax - xmin, ymax - ymin, zmax - zmin };
}

// Compute volume via mass properties
inline double volume(const TopoDS_Shape& shape)
{
  GProp_GProps props; BRepGProp::VolumeProperties(shape, props); return props.Mass();
}

// Count primary surface types across faces
struct SurfaceTypeCounts { int planar = 0; int cylindrical = 0; };
inline SurfaceTypeCounts countSurfaceTypes(const TopoDS_Shape& shape)
{
  SurfaceTypeCounts c{};
  for (TopExp_Explorer exp(shape, TopAbs_FACE); exp.More(); exp.Next())
  {
    const TopoDS_Face& f = TopoDS::Face(exp.Current());
    Handle(Geom_Surface) s = BRep_Tool::Surface(f);
    GeomAdaptor_Surface ga(s);
    switch (ga.GetType())
    {
      case GeomAbs_Plane:     ++c.planar; break;
      case GeomAbs_Cylinder:  ++c.cylindrical; break;
      default: break;
    }
  }
  return c;
}

