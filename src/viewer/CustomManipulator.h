#pragma once

#include <AIS_Manipulator.hxx>

// Custom manipulator: identical to default, but hides any visual parts
// related to Scaling and TranslationPlane, regardless of internal skin logic.
class CustomManipulator : public AIS_Manipulator
{
  DEFINE_STANDARD_RTTIEXT(CustomManipulator, AIS_Manipulator)
public:
  CustomManipulator() = default;
  explicit CustomManipulator(const gp_Ax2& thePos) : AIS_Manipulator(thePos) {}

protected:
  // After base presentation is built, clear groups for unwanted modes
  void Compute(const Handle(PrsMgr_PresentationManager)& thePrsMgr,
               const Handle(Prs3d_Presentation)&         thePrs,
               const Standard_Integer                    theMode) override;
};

