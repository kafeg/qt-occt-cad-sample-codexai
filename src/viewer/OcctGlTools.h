#pragma once

#include <OpenGl_Context.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <V3d_View.hxx>

namespace OcctGlTools
{
  inline Handle(OpenGl_Context) GetGlContext(const Handle(V3d_View)& theView)
  {
    Handle(Graphic3d_GraphicDriver) aDrv = theView->Viewer()->Driver();
    Handle(OpenGl_GraphicDriver)     aGl  = Handle(OpenGl_GraphicDriver)::DownCast(aDrv);
    return !aGl.IsNull() ? aGl->GetSharedContext() : Handle(OpenGl_Context)();
  }
}
