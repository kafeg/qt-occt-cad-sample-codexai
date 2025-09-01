#pragma once

#include "Feature.h"
#include <Standard_DefineHandle.hxx>

class CylinderFeature;
DEFINE_STANDARD_HANDLE(CylinderFeature, Feature)

// Cylinder primitive feature: radius (R) and height (H)
class CylinderFeature : public Feature
{
  DEFINE_STANDARD_RTTIEXT(CylinderFeature, Feature)

public:
  CylinderFeature() = default;
  CylinderFeature(double radius, double height) { set(radius, height); }

  void set(double radius, double height)
  {
    setRadius(radius);
    setHeight(height);
  }

  void setRadius(double r) { params()[Feature::ParamKey::Radius] = r; }
  void setHeight(double h) { params()[Feature::ParamKey::Height] = h; }

  double radius() const;
  double height() const;

  void execute() override;

  // DocumentItem
  Kind kind() const override { return Kind::CylinderFeature; }
};
