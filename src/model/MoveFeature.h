#pragma once

#include "Feature.h"
#include <Standard_DefineHandle.hxx>
#include <DocumentItem.h>

class MoveFeature;
DEFINE_STANDARD_HANDLE(MoveFeature, Feature)

// Move feature: applies a rigid transform (T + R) to a source Feature's shape
class MoveFeature : public Feature
{
  DEFINE_STANDARD_RTTIEXT(MoveFeature, Feature)

public:
  MoveFeature() = default;

  // Construct with explicit source link and params
  MoveFeature(DocumentItem::Id sourceId,
              double tx, double ty, double tz,
              double rxDeg, double ryDeg, double rzDeg)
    : m_sourceId(sourceId)
  {
    setTranslation(tx, ty, tz);
    setRotation(rxDeg, ryDeg, rzDeg);
  }

  // Runtime linkage helpers
  void setSource(const Handle(Feature)& src) { m_source = src; }
  Handle(Feature) source() const { return m_source; }

  void setSourceId(DocumentItem::Id id) { m_sourceId = id; }
  DocumentItem::Id sourceId() const { return m_sourceId; }

  // Param setters/getters
  void setTranslation(double tx, double ty, double tz)
  {
    params()[Feature::ParamKey::Tx] = tx;
    params()[Feature::ParamKey::Ty] = ty;
    params()[Feature::ParamKey::Tz] = tz;
  }
  void setRotation(double rxDeg, double ryDeg, double rzDeg)
  {
    params()[Feature::ParamKey::Rx] = rxDeg;
    params()[Feature::ParamKey::Ry] = ryDeg;
    params()[Feature::ParamKey::Rz] = rzDeg;
  }

  double tx() const { return Feature::paramAsDouble(params(), Feature::ParamKey::Tx, 0.0); }
  double ty() const { return Feature::paramAsDouble(params(), Feature::ParamKey::Ty, 0.0); }
  double tz() const { return Feature::paramAsDouble(params(), Feature::ParamKey::Tz, 0.0); }
  double rxDeg() const { return Feature::paramAsDouble(params(), Feature::ParamKey::Rx, 0.0); }
  double ryDeg() const { return Feature::paramAsDouble(params(), Feature::ParamKey::Ry, 0.0); }
  double rzDeg() const { return Feature::paramAsDouble(params(), Feature::ParamKey::Rz, 0.0); }

  void execute() override;

public:
  // DocumentItem
  Kind kind() const override { return Kind::MoveFeature; }
  std::string serialize() const override;
  void        deserialize(const std::string& data) override;

private:
  Handle(Feature)  m_source;   // runtime resolved source feature (optional)
  DocumentItem::Id m_sourceId{0};
};

