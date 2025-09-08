#pragma once

#include "Feature.h"
#include <Standard_DefineHandle.hxx>

#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
#include <Quantity_Color.hxx>

class AIS_Shape; // fwd decl to avoid heavy include in header

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
  void  setTransparency(double t) { params()[Feature::ParamKey::Transparency] = t; }
  double transparency() const;

  // Feature API
  void execute() override;

  // Default visual style for all datum planes
  static Quantity_Color defaultColor()
  {
    // Unified orange tone in sRGB
    return Quantity_Color(Quantity_NOC_ORANGE);
  }

  // Apply visual style to an AIS shape (color + transparency)
  void applyStyle(const Handle(AIS_Shape)& ais) const;

  // DocumentItem
  Kind kind() const override { return Kind::PlaneFeature; }
};
