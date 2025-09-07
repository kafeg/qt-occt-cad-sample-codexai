#pragma once

#include "Feature.h"
#include <Standard_DefineHandle.hxx>

#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>

class AxeFeature;
DEFINE_STANDARD_HANDLE(AxeFeature, Feature)

// Datum axis feature: line defined by origin, direction and length
// - Origin: axis center point in world coordinates
// - Direction: axis direction (normalized on execute)
// - Length: half-length extent in each direction
class AxeFeature : public Feature
{
  DEFINE_STANDARD_RTTIEXT(AxeFeature, Feature)

public:
  AxeFeature() = default;

  AxeFeature(const gp_Pnt& origin, const gp_Dir& dir, double length)
  {
    setOrigin(origin);
    setDirection(dir);
    setLength(length);
  }

  // Param setters
  void setOrigin(const gp_Pnt& p)
  {
    params()[Feature::ParamKey::Ox] = p.X();
    params()[Feature::ParamKey::Oy] = p.Y();
    params()[Feature::ParamKey::Oz] = p.Z();
  }

  void setDirection(const gp_Dir& d)
  {
    params()[Feature::ParamKey::Nx] = d.X();
    params()[Feature::ParamKey::Ny] = d.Y();
    params()[Feature::ParamKey::Nz] = d.Z();
  }

  void setLength(double l) { params()[Feature::ParamKey::Size] = l; }

  // Param getters
  gp_Pnt origin() const;
  gp_Dir direction() const;
  double length() const;

  // Modifiers
  void setFixedGeometry(bool on) { params()[Feature::ParamKey::FixedGeometry] = on ? 1 : 0; }

  bool isFixedGeometry() const
  {
    return Feature::paramAsDouble(params(), Feature::ParamKey::FixedGeometry, 0.0) != 0.0;
  }

  // Feature API
  void execute() override;

  // DocumentItem
  Kind kind() const override { return Kind::AxeFeature; }
};
