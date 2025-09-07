#include "AxeFeature.h"
#include <DocumentItem.h>

#include <BRepBuilderAPI_MakeEdge.hxx>
#include <gp_Vec.hxx>

IMPLEMENT_STANDARD_RTTIEXT(AxeFeature, Feature)

namespace
{
const bool kAxeFeatureReg = []() {
  DocumentItem::registerFactory(DocumentItem::Kind::AxeFeature,
                                []() { return std::shared_ptr<DocumentItem>(new AxeFeature()); });
  return true;
}();
} // namespace

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
  const double l = Feature::paramAsDouble(params(), Feature::ParamKey::Size, 100.0);
  return l <= 0.0 ? 100.0 : l;
}

void AxeFeature::execute()
{
  const gp_Pnt o = origin();
  const gp_Dir d = direction();
  const double l = length();
  gp_Vec       v(d.XYZ());
  v.Multiply(l);
  const gp_Pnt p1 = o.Translated(-v);
  const gp_Pnt p2 = o.Translated(v);
  m_shape         = BRepBuilderAPI_MakeEdge(p1, p2).Shape();
}
