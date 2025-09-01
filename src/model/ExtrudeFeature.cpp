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


double ExtrudeFeature::distance() const
{
  return Feature::paramAsDouble(params(), Feature::ParamKey::Distance, 0.0);
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
