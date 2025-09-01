#include "CylinderFeature.h"

#include <KernelAPI.h>
#include <variant>

IMPLEMENT_STANDARD_RTTIEXT(CylinderFeature, Feature)

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

double CylinderFeature::radius() const
{
  return paramAsDouble(params(), Feature::ParamKey::Radius, 0.0);
}

double CylinderFeature::height() const
{
  return paramAsDouble(params(), Feature::ParamKey::Height, 0.0);
}

void CylinderFeature::execute()
{
  m_shape = KernelAPI::makeCylinder(radius(), height());
}
