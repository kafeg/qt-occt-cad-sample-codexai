#include "MoveFeature.h"

#include <DocumentItem.h>
#include <BRepBuilderAPI_Transform.hxx>
#include <gp_Ax1.hxx>
#include <gp_Trsf.hxx>
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

  // Build transform: apply rotations around X, Y, Z (degrees), then translation
  gp_Trsf trsf; // identity
  const double rx = rxDeg() * (M_PI / 180.0);
  const double ry = ryDeg() * (M_PI / 180.0);
  const double rz = rzDeg() * (M_PI / 180.0);
  if (std::abs(rx) > 0.0)
  {
    gp_Trsf r; r.SetRotation(gp_Ax1(gp_Pnt(0,0,0), gp::DX()), rx); trsf.Multiply(r);
  }
  if (std::abs(ry) > 0.0)
  {
    gp_Trsf r; r.SetRotation(gp_Ax1(gp_Pnt(0,0,0), gp::DY()), ry); trsf.Multiply(r);
  }
  if (std::abs(rz) > 0.0)
  {
    gp_Trsf r; r.SetRotation(gp_Ax1(gp_Pnt(0,0,0), gp::DZ()), rz); trsf.Multiply(r);
  }
  if (std::abs(tx()) > 0.0 || std::abs(ty()) > 0.0 || std::abs(tz()) > 0.0)
  {
    gp_Trsf t; t.SetTranslation(gp_Vec(tx(), ty(), tz())); trsf.Multiply(t);
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
