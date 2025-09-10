#include "TestShapes.h"
#include "../core/KernelAPI.h"
#include <BRepPrimAPI_MakeSphere.hxx>

namespace TestShapes
{
  TopoDS_Shape createTestBox()
  {
    return KernelAPI::makeBox(50.0, 50.0, 50.0);
  }
  
  TopoDS_Shape createTestCylinder()
  {
    return KernelAPI::makeCylinder(25.0, 50.0);
  }
  
  TopoDS_Shape createTestSphere()
  {
    return BRepPrimAPI_MakeSphere(30.0).Shape();
  }
}
