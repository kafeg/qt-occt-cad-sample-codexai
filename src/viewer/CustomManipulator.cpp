#include "CustomManipulator.h"

#include <Graphic3d_Group.hxx>

IMPLEMENT_STANDARD_RTTIEXT(CustomManipulator, AIS_Manipulator)

void CustomManipulator::Compute(const Handle(PrsMgr_PresentationManager)& thePrsMgr,
                                const Handle(Prs3d_Presentation)&         thePrs,
                                const Standard_Integer                    theMode)
{
  AIS_Manipulator::Compute(thePrsMgr, thePrs, theMode);

  // Proactively clear any groups belonging to unwanted modes
  for (int axis = 0; axis < 3; ++axis)
  {
    if (Handle(Graphic3d_Group) gS = getGroup(axis, AIS_MM_Scaling))
      gS->Clear();
    if (Handle(Graphic3d_Group) gP = getGroup(axis, AIS_MM_TranslationPlane))
      gP->Clear();
  }
  // Some skins may add common (non-axis) artifacts; try axis index 3 as a fallback if present
  if (Handle(Graphic3d_Group) gS = getGroup(3, AIS_MM_Scaling))
    gS->Clear();
  if (Handle(Graphic3d_Group) gP = getGroup(3, AIS_MM_TranslationPlane))
    gP->Clear();
}

