#include "ExtrudeFeature.h"

#include <KernelAPI.h>
#include <Sketch.h>
#include <variant>
#include <sstream>

IMPLEMENT_STANDARD_RTTIEXT(ExtrudeFeature, Feature)

namespace {
const bool kExtrudeFeatureReg = [](){
  DocumentItem::registerFactory(DocumentItem::Kind::ExtrudeFeature, [](){ return std::shared_ptr<DocumentItem>(new ExtrudeFeature()); });
  return true;
}();
}

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

double ExtrudeFeature::distance() const
{
  return paramAsDouble(params(), Feature::ParamKey::Distance, 0.0);
}

void ExtrudeFeature::execute()
{
  if (!m_sketch)
  {
    m_shape = TopoDS_Shape();
    return;
  }
  const auto wires = m_sketch->toOcctWires();
  m_shape = KernelAPI::extrude(wires, distance());
}

// Append base Feature encoding + extrude-specific fields
std::string ExtrudeFeature::serialize() const
{
  std::ostringstream os;
  os << Feature::serialize();
  os << "sketchId=" << m_sketchId << "\n";
  return os.str();
}

void ExtrudeFeature::deserialize(const std::string& data)
{
  Feature::deserialize(data);
  std::size_t pos = 0;
  while (pos < data.size())
  {
    std::size_t eol = data.find('\n', pos);
    std::string line = data.substr(pos, eol == std::string::npos ? std::string::npos : eol - pos);
    pos = (eol == std::string::npos) ? data.size() : eol + 1;
    if (line.rfind("sketchId=", 0) == 0)
    {
      m_sketchId = static_cast<DocumentItem::Id>(std::stoull(line.substr(9)));
    }
  }
}
