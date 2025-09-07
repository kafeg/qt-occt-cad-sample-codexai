#pragma once

#include "Feature.h"
#include <Standard_DefineHandle.hxx>

#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>

class PlaneFeature;
DEFINE_STANDARD_HANDLE(PlaneFeature, Feature)

// Datum plane feature: oriented rectangular finite plane defined by origin, normal and size
// - Origin: plane center point in world coords
// - Normal: plane normal vector (normalized on execute)
// - Size:   half-extent U/V (square of 2*size x 2*size)
class PlaneFeature : public Feature
{
  DEFINE_STANDARD_RTTIEXT(PlaneFeature, Feature)

public:
  PlaneFeature() = default;

  PlaneFeature(const gp_Pnt& origin, const gp_Dir& normal, double size)
  {
    setOrigin(origin);
    setNormal(normal);
    setSize(size);
  }

  // Param setters
  void setOrigin(const gp_Pnt& p)
  {
    params()[Feature::ParamKey::Ox] = p.X();
    params()[Feature::ParamKey::Oy] = p.Y();
    params()[Feature::ParamKey::Oz] = p.Z();
  }
  void setNormal(const gp_Dir& n)
  {
    params()[Feature::ParamKey::Nx] = n.X();
    params()[Feature::ParamKey::Ny] = n.Y();
    params()[Feature::ParamKey::Nz] = n.Z();
  }
  void setSize(double s) { params()[Feature::ParamKey::Size] = s; }

  // Param getters
  gp_Pnt origin() const;
  gp_Dir normal() const;
  double size() const;

  // Modifiers
  void  setFixed(bool on) { params()[Feature::ParamKey::Fixed] = on ? 1 : 0; }
  bool  isFixed() const { return Feature::paramAsDouble(params(), Feature::ParamKey::Fixed, 0.0) != 0.0; }
  void  setTransparency(double t) { params()[Feature::ParamKey::Transparency] = t; }
  double transparency() const;

  // Feature API
  void execute() override;

  // DocumentItem
  Kind kind() const override { return Kind::PlaneFeature; }
};
