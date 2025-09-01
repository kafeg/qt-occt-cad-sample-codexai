#include "MoveFeature.h"

#include <DocumentItem.h>
#include <BRepBuilderAPI_Transform.hxx>
#include <gp_Ax1.hxx>
#include <gp_Trsf.hxx>
#include <gp_Quaternion.hxx>
#include <gp_EulerSequence.hxx>
#include <gp.hxx>

#include <sstream>
#include <cmath>

IMPLEMENT_STANDARD_RTTIEXT(MoveFeature, Feature)

namespace {
const bool kMoveFeatureReg = [](){
  DocumentItem::registerFactory(DocumentItem::Kind::MoveFeature, [](){ return std::shared_ptr<DocumentItem>(new MoveFeature()); });
  return true;
}();
}

void MoveFeature::execute()
{
  if (m_source.IsNull())
  {
    m_shape = TopoDS_Shape();
    return;
  }

  // If precise delta is provided (from manipulator), use it as-is to avoid
  // ambiguities of Euler angle decomposition for combined rotations.
  gp_Trsf trsf;
  if (m_delta.Form() != gp_Identity)
  {
    trsf = m_delta;
  }
  else
  {
    // Fallback: rebuild from params (Euler XYZ + T)
    const double rx = rxDeg() * (M_PI / 180.0);
    const double ry = ryDeg() * (M_PI / 180.0);
    const double rz = rzDeg() * (M_PI / 180.0);
    gp_Quaternion q; q.SetEulerAngles(gp_Intrinsic_XYZ, rx, ry, rz);
    trsf.SetTransformation(q, gp_Vec(tx(), ty(), tz()));
  }

  BRepBuilderAPI_Transform tr(m_source->shape(), trsf, true);
  m_shape = tr.Shape();
}

// Append base Feature encoding + move-specific fields
std::string MoveFeature::serialize() const
{
  std::ostringstream os;
  os << Feature::serialize();
  os << "sourceId=" << m_sourceId << "\n";
  return os.str();
}

void MoveFeature::deserialize(const std::string& data)
{
  Feature::deserialize(data);
  std::size_t pos = 0;
  while (pos < data.size())
  {
    std::size_t eol = data.find('\n', pos);
    std::string line = data.substr(pos, eol == std::string::npos ? std::string::npos : eol - pos);
    pos = (eol == std::string::npos) ? data.size() : eol + 1;
    if (line.rfind("sourceId=", 0) == 0)
    {
      m_sourceId = static_cast<DocumentItem::Id>(std::stoull(line.substr(9)));
    }
  }
}
