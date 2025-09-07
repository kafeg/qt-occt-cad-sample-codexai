#include "PointFeature.h"
#include <DocumentItem.h>

#include <BRepPrimAPI_MakeSphere.hxx>

IMPLEMENT_STANDARD_RTTIEXT(PointFeature, Feature)

namespace {
const bool kPointFeatureReg = [](){
  DocumentItem::registerFactory(DocumentItem::Kind::PointFeature, [](){ return std::shared_ptr<DocumentItem>(new PointFeature()); });
  return true;
}();
}

gp_Pnt PointFeature::origin() const
{
  const double x = Feature::paramAsDouble(params(), Feature::ParamKey::Ox, 0.0);
  const double y = Feature::paramAsDouble(params(), Feature::ParamKey::Oy, 0.0);
  const double z = Feature::paramAsDouble(params(), Feature::ParamKey::Oz, 0.0);
  return gp_Pnt(x, y, z);
}

double PointFeature::radius() const
{
  const double r = Feature::paramAsDouble(params(), Feature::ParamKey::Radius, 10.0);
  return r <= 0.0 ? 10.0 : r;
}

double PointFeature::transparency() const
{
  return Feature::paramAsDouble(params(), Feature::ParamKey::Transparency, 0.0);
}

void PointFeature::execute()
{
  const gp_Pnt o = origin();
  const double r = radius();
  // Represent the point as a small sphere for robust shading/selection
  m_shape = BRepPrimAPI_MakeSphere(o, r).Shape();
}

