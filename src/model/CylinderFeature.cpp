#include "CylinderFeature.h"
#include <DocumentItem.h>

#include <KernelAPI.h>
#include <variant>

IMPLEMENT_STANDARD_RTTIEXT(CylinderFeature, Feature)

namespace {
const bool kCylinderFeatureReg = [](){
  DocumentItem::registerFactory(DocumentItem::Kind::CylinderFeature, [](){ return std::shared_ptr<DocumentItem>(new CylinderFeature()); });
  return true;
}();
}


double CylinderFeature::radius() const
{
  return Feature::paramAsDouble(params(), Feature::ParamKey::Radius, 0.0);
}

double CylinderFeature::height() const
{
  return Feature::paramAsDouble(params(), Feature::ParamKey::Height, 0.0);
}

void CylinderFeature::execute()
{
  m_shape = KernelAPI::makeCylinder(radius(), height());
}
