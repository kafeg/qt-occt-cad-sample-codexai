#include "AxeFeature.h"
#include <DocumentItem.h>

#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRep_Builder.hxx>
#include <TopoDS_Compound.hxx>
#include <gp_Vec.hxx>
#include <gp_Ax2.hxx>
#include <algorithm>

IMPLEMENT_STANDARD_RTTIEXT(AxeFeature, Feature)

namespace {
const bool kAxeFeatureReg = [](){
  DocumentItem::registerFactory(DocumentItem::Kind::AxeFeature, [](){ return std::shared_ptr<DocumentItem>(new AxeFeature()); });
  return true;
}();
}

gp_Pnt AxeFeature::origin() const
{
  const double x = Feature::paramAsDouble(params(), Feature::ParamKey::Ox, 0.0);
  const double y = Feature::paramAsDouble(params(), Feature::ParamKey::Oy, 0.0);
  const double z = Feature::paramAsDouble(params(), Feature::ParamKey::Oz, 0.0);
  return gp_Pnt(x, y, z);
}

gp_Dir AxeFeature::direction() const
{
  gp_Vec v(Feature::paramAsDouble(params(), Feature::ParamKey::Nx, 1.0),
           Feature::paramAsDouble(params(), Feature::ParamKey::Ny, 0.0),
           Feature::paramAsDouble(params(), Feature::ParamKey::Nz, 0.0));
  if (v.SquareMagnitude() <= gp::Resolution())
    v = gp_Vec(1.0, 0.0, 0.0);
  return gp_Dir(v);
}

double AxeFeature::length() const
{
  const double l = Feature::paramAsDouble(params(), Feature::ParamKey::Length, 100.0);
  return l <= 0.0 ? 100.0 : l;
}

void AxeFeature::execute()
{
  const gp_Pnt o = origin();
  const gp_Dir d = direction();
  const double l = length();

  // Build a compound of short cylinder segments along the axis to emulate
  // a dash-dot line: [dash]-gap-[dot]-gap-...
  // Geometry thickness now comes from cylinder radius.
  // Visual pattern tuning
  const Standard_Real kDashLen = 22.0; // longer dashes
  const Standard_Real kDotLen  = 5.0;  // longer dots
  const Standard_Real kBaseGap = 8.0;  // slightly larger base gap
  const Standard_Real kRadius  = 3.0; // thicker for better visibility
  const Standard_Real kOriginOffsetMin = 30.0; // leave a small blank near the origin

  BRep_Builder builder;
  TopoDS_Compound comp;
  builder.MakeCompound(comp);

  gp_Vec vDir(d.XYZ()); // unit-length vector in axis direction
  // Add an initial offset from origin to avoid segments touching the origin
  const Standard_Real originOffset = std::max(kOriginOffsetMin, 2.0 * kRadius);
  Standard_Real pos = originOffset;
  bool placeDash = true; // alternate dash and dot segments

  while (pos < l - gp::Resolution())
  {
    // Advance with either a dash or a dot
    const Standard_Real segTarget = placeDash ? kDashLen : kDotLen;
    const Standard_Real segLen = std::min(segTarget, l - pos);
    if (segLen > gp::Resolution())
    {
      const gp_Pnt segStart = o.Translated(vDir.Multiplied(pos));
      const gp_Ax2 ax(segStart, d);
      TopoDS_Shape segCyl = BRepPrimAPI_MakeCylinder(ax, kRadius, segLen);
      builder.Add(comp, segCyl);
      pos += segLen;
    }

    // Add a gap between segments if room remains
    if (pos < l - gp::Resolution())
    {
      const Standard_Real gapLen = std::max(kBaseGap, 3.5 * kRadius);
      pos += gapLen;
    }

    placeDash = !placeDash; // alternate dash/dot
  }

  m_shape = comp;
}
