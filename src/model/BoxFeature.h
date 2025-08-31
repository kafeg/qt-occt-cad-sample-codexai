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
  BoxFeature(double dx, double dy, double dz)
    : myDx(dx), myDy(dy), myDz(dz) {}

  void setSize(double dx, double dy, double dz) { myDx = dx; myDy = dy; myDz = dz; }
  void setDx(double dx) { myDx = dx; }
  void setDy(double dy) { myDy = dy; }
  void setDz(double dz) { myDz = dz; }

  double dx() const { return myDx; }
  double dy() const { return myDy; }
  double dz() const { return myDz; }

  // Feature API
  void execute() override;

private:
  double myDx{0.0};
  double myDy{0.0};
  double myDz{0.0};
};
