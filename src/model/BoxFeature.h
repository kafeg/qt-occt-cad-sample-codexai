#pragma once

#include "Feature.h"
#include <Standard_DefineHandle.hxx>

class BoxFeature;
DEFINE_STANDARD_HANDLE(BoxFeature, Feature)

// Box primitive feature storing its dimensions
class BoxFeature : public Feature
{
  DEFINE_STANDARD_RTTIEXT(BoxFeature, Feature)

public:
  BoxFeature() = default;

  BoxFeature(double dx, double dy, double dz) { setSize(dx, dy, dz); }

  // Parameter accessors (backed by Feature::params())
  void setSize(double dx, double dy, double dz)
  {
    setDx(dx);
    setDy(dy);
    setDz(dz);
  }

  void setDx(double dx) { params()[Feature::ParamKey::Dx] = dx; }

  void setDy(double dy) { params()[Feature::ParamKey::Dy] = dy; }

  void setDz(double dz) { params()[Feature::ParamKey::Dz] = dz; }

  double dx() const;
  double dy() const;
  double dz() const;

  // Feature API
  void execute() override;
};
