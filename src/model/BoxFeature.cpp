#include "BoxFeature.h"
#include <DocumentItem.h>

#include <KernelAPI.h>
#include <variant>

IMPLEMENT_STANDARD_RTTIEXT(BoxFeature, Feature)

namespace {
const bool kBoxFeatureReg = [](){
  DocumentItem::registerFactory(DocumentItem::Kind::BoxFeature, [](){ return std::shared_ptr<DocumentItem>(new BoxFeature()); });
  return true;
}();
}


double BoxFeature::dx() const
{
  return Feature::paramAsDouble(params(), Feature::ParamKey::Dx, 0.0);
}

double BoxFeature::dy() const
{
  return Feature::paramAsDouble(params(), Feature::ParamKey::Dy, 0.0);
}

double BoxFeature::dz() const
{
  return Feature::paramAsDouble(params(), Feature::ParamKey::Dz, 0.0);
}

void BoxFeature::execute()
{
  m_shape = KernelAPI::makeBox(dx(), dy(), dz());
}
