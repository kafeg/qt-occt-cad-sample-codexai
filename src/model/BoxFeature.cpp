#include "BoxFeature.h"

#include <KernelAPI.h>

IMPLEMENT_STANDARD_RTTIEXT(BoxFeature, Feature)

void BoxFeature::execute()
{
  myShape = KernelAPI::makeBox(myDx, myDy, myDz);
}
