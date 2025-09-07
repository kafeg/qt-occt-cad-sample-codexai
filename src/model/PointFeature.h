#pragma once

#include "Feature.h"
#include <Standard_DefineHandle.hxx>

#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>

class PointFeature;
DEFINE_STANDARD_HANDLE(PointFeature, Feature)

// Datum point feature: small sphere at given origin (world coords)
// - Origin: point center in world coords (Ox/Oy/Oz)
// - Radius: visual radius (uses Feature::ParamKey::Radius)
class PointFeature : public Feature
{
  DEFINE_STANDARD_RTTIEXT(PointFeature, Feature)

public:
  PointFeature() = default;

  explicit PointFeature(const gp_Pnt& origin, double radius = 10.0)
  {
    setOrigin(origin);
    setRadius(radius);
  }

  // Param setters
  void setOrigin(const gp_Pnt& p)
  {
    params()[Feature::ParamKey::Ox] = p.X();
    params()[Feature::ParamKey::Oy] = p.Y();
    params()[Feature::ParamKey::Oz] = p.Z();
  }
  void setRadius(double r) { params()[Feature::ParamKey::Radius] = r; }

  // Param getters
  gp_Pnt origin() const;
  double radius() const;

  // Modifiers
  void  setFixedGeometry(bool on) { params()[Feature::ParamKey::FixedGeometry] = on ? 1 : 0; }
  bool  isFixedGeometry() const { return Feature::paramAsDouble(params(), Feature::ParamKey::FixedGeometry, 0.0) != 0.0; }
  void  setTransparency(double t) { params()[Feature::ParamKey::Transparency] = t; }
  double transparency() const;

  // Feature API
  void execute() override;

  // DocumentItem
  Kind kind() const override { return Kind::PointFeature; }
};

