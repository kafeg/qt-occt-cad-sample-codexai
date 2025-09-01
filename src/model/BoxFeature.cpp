#include "BoxFeature.h"

#include <KernelAPI.h>
#include <variant>

IMPLEMENT_STANDARD_RTTIEXT(BoxFeature, Feature)

namespace
{
static double paramAsDouble(const Feature::ParamMap& pm, Feature::ParamKey key, double defVal)
{
  auto it = pm.find(key);
  if (it == pm.end())
    return defVal;
  const Feature::ParamValue& v = it->second;
  if (std::holds_alternative<double>(v))
    return std::get<double>(v);
  if (std::holds_alternative<int>(v))
    return static_cast<double>(std::get<int>(v));
  return defVal;
}
} // namespace

double BoxFeature::dx() const
{
  return paramAsDouble(params(), Feature::ParamKey::Dx, 0.0);
}

double BoxFeature::dy() const
{
  return paramAsDouble(params(), Feature::ParamKey::Dy, 0.0);
}

double BoxFeature::dz() const
{
  return paramAsDouble(params(), Feature::ParamKey::Dz, 0.0);
}

void BoxFeature::execute()
{
  m_shape = KernelAPI::makeBox(dx(), dy(), dz());
}
