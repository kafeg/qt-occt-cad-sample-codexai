#include "InfiniteGrid.h"

#include <Prs3d_Root.hxx>
#include <Prs3d_Presentation.hxx>
#include <Graphic3d_ArrayOfSegments.hxx>
#include <Graphic3d_AspectLine3d.hxx>
#include <Aspect_NeutralWindow.hxx>
#include <Quantity_Color.hxx>
#include <gp_Pnt.hxx>

IMPLEMENT_STANDARD_RTTIEXT(InfiniteGrid, AIS_InteractiveObject)

namespace {
static inline Quantity_Color kGridMinorColor() { return Quantity_Color(Quantity_NOC_GRAY40); }
static inline Quantity_Color kGridMajorColor() { return Quantity_Color(Quantity_NOC_GRAY20); }
static inline Quantity_Color kAxesXColor()     { return Quantity_Color(1.00, 0.15, 0.15, Quantity_TOC_sRGB); }
static inline Quantity_Color kAxesYColor()     { return Quantity_Color(0.20, 0.75, 0.20, Quantity_TOC_sRGB); }
static inline Quantity_Color kOriginColor()    { return Quantity_Color(0.10, 0.10, 0.10, Quantity_TOC_sRGB); }
}

void InfiniteGrid::updateFromView(const Handle(V3d_View)& theView)
{
  if (theView.IsNull()) return;
  Handle(Aspect_NeutralWindow) aWnd = Handle(Aspect_NeutralWindow)::DownCast(theView->Window());
  Standard_Integer vpW = 0, vpH = 0;
  if (!aWnd.IsNull())
  {
    aWnd->Size(vpW, vpH);
  }
  if (vpW <= 0 || vpH <= 0)
  {
    // Fallback viewport for headless contexts
    vpW = 800; vpH = 600;
  }

  // Sample world distance corresponding to 20 horizontal pixels at viewport center
  const int cx = vpW / 2; const int cy = vpH / 2; gp_Pnt p0, p1;
  if (!rayHitZ0(theView, cx, cy, p0) || !rayHitZ0(theView, cx + 20, cy, p1))
  {
    // If projection is parallel to XY, fallback to default step/extents
    if (m_Step <= 0.0) m_Step = 10.0;
    const Standard_Real span = 20.0 * m_Step;
    m_XMin = -span; m_XMax = span; m_YMin = -span; m_YMax = span;
    m_Initialized = Standard_True;
    SetToUpdate();
    return;
  }
  {
    const Standard_Real worldPer20px = p0.Distance(p1);
    computeStepFromWorldPer20px(vpW, vpH, worldPer20px);
  }
  computeExtentsFromView(theView, vpW, vpH);
  SetToUpdate();
}

void InfiniteGrid::updateFromViewportSample(Standard_Integer theVpW, Standard_Integer theVpH, Standard_Real theWorldPer20px)
{
  computeStepFromWorldPer20px(theVpW, theVpH, theWorldPer20px);
  // Extents depend on view framing; without a view, derive generous extents covering viewport
  // Assuming a typical perspective, span ~12 steps around origin as a safe default
  const Standard_Real marginSteps = 12.0;
  const Standard_Real span = m_Step * marginSteps;
  m_XMin = -span; m_XMax = span;
  m_YMin = -span; m_YMax = span;
  m_Initialized = Standard_True;
  SetToUpdate();
}

void InfiniteGrid::computeStepFromWorldPer20px(Standard_Real, Standard_Real, Standard_Real theWorldPer20px)
{
  // Derive minor grid step from a 20px sample using a 1–2–5*10^n series.
  // Aim ~20px per cell: snap worldPer20px to nearest {1,2,5,10}*10^n.
  if (!(theWorldPer20px > 0.0) || !std::isfinite((double)theWorldPer20px))
  {
    if (m_Step <= 0.0) m_Step = 1.0;
    return;
  }

  const Standard_Real x = theWorldPer20px;
  const Standard_Real n = Floor(Log10(x));
  const Standard_Real base = Pow(10.0, n);
  const Standard_Real r = x / base; // in [1,10)

  Standard_Real mantissa = 1.0;
  if (r < 1.5)      mantissa = 1.0;
  else if (r < 3.0) mantissa = 2.0;
  else if (r < 7.0) mantissa = 5.0;
  else              mantissa = 10.0;

  m_Step = mantissa * base;
  if (!(m_Step > 0.0) || !std::isfinite((double)m_Step))
  {
    m_Step = 1.0;
  }
}

void InfiniteGrid::computeExtentsFromView(const Handle(V3d_View)& theView, Standard_Integer theVpW, Standard_Integer theVpH)
{
  if (theView.IsNull()) return;
  // Project viewport corners to Z=0
  gp_Pnt p00, p10, p11, p01;
  if (!rayHitZ0(theView, 0,        0,        p00)
   || !rayHitZ0(theView, theVpW-1, 0,        p10)
   || !rayHitZ0(theView, theVpW-1, theVpH-1, p11)
   || !rayHitZ0(theView, 0,        theVpH-1, p01))
  {
    // Fallback extents if projection fails (edge-on)
    const Standard_Real span = 20.0 * Max(m_Step, 1.0);
    m_XMin = -span; m_XMax = span; m_YMin = -span; m_YMax = span;
    m_Initialized = Standard_True;
    return;
  }
  Standard_Real xmin = p00.X(), xmax = p00.X();
  Standard_Real ymin = p00.Y(), ymax = p00.Y();
  auto grow = [&](const gp_Pnt& p){ xmin = Min(xmin, p.X()); xmax = Max(xmax, p.X()); ymin = Min(ymin, p.Y()); ymax = Max(ymax, p.Y()); };
  grow(p10); grow(p11); grow(p01);
  // Add a margin of 2 steps beyond viewport on all sides
  const Standard_Real m = 2.0 * m_Step;
  m_XMin = floor((xmin - m) / m_Step) * m_Step;
  m_XMax = ceil ((xmax + m) / m_Step) * m_Step;
  m_YMin = floor((ymin - m) / m_Step) * m_Step;
  m_YMax = ceil ((ymax + m) / m_Step) * m_Step;
  m_Initialized = Standard_True;
}

Standard_Boolean InfiniteGrid::rayHitZ0(const Handle(V3d_View)& theView, int thePx, int thePy, gp_Pnt& theHit)
{
  if (theView.IsNull()) return Standard_False;
  Standard_Real X = 0.0, Y = 0.0, Z = 0.0;
  Standard_Real Vx = 0.0, Vy = 0.0, Vz = 0.0;
  theView->ConvertWithProj(thePx, thePy, X, Y, Z, Vx, Vy, Vz);
  if (Abs(Vz) < 1.0e-12) return Standard_False;
  const Standard_Real t = -Z / Vz;
  theHit.SetCoord(X + t * Vx, Y + t * Vy, 0.0);
  return Standard_True;
}

void InfiniteGrid::Compute(const Handle(PrsMgr_PresentationManager3d)&,
                           const Handle(Prs3d_Presentation)&            thePrs,
                           const Standard_Integer)
{
  thePrs->Clear();
  if (!m_Initialized || m_Step <= 0.0) return;

  // Build grid segments
  const Standard_Integer x0 = (Standard_Integer)Floor(m_XMin / m_Step);
  const Standard_Integer x1 = (Standard_Integer)ceil (m_XMax / m_Step);
  const Standard_Integer y0 = (Standard_Integer)Floor(m_YMin / m_Step);
  const Standard_Integer y1 = (Standard_Integer)ceil (m_YMax / m_Step);

  const Standard_Integer nbX = Max(0, x1 - x0 + 1);
  const Standard_Integer nbY = Max(0, y1 - y0 + 1);
  const Standard_Integer gridSegs = nbX + nbY;
  if (gridSegs > 0)
  {
    // Separate minor and major lines (every 10th) for better visibility
    Handle(Graphic3d_ArrayOfSegments) segMinor = new Graphic3d_ArrayOfSegments(2 * gridSegs, gridSegs);
    Handle(Graphic3d_ArrayOfSegments) segMajor = new Graphic3d_ArrayOfSegments(2 * gridSegs / 10 + 4, gridSegs / 10 + 2);

    auto isMajor = [](Standard_Integer i) {
      Standard_Integer m = i % 10; if (m < 0) m += 10; return m == 0; };

    // Vertical lines (constant X)
    for (Standard_Integer ix = x0; ix <= x1; ++ix)
    {
      const Standard_Real x = ix * m_Step;
      Handle(Graphic3d_ArrayOfSegments)& target = isMajor(ix) ? segMajor : segMinor;
      target->AddVertex((Standard_ShortReal)x, (Standard_ShortReal)m_YMin, 0.0f);
      target->AddVertex((Standard_ShortReal)x, (Standard_ShortReal)m_YMax, 0.0f);
      target->AddEdge(2);
    }
    // Horizontal lines (constant Y)
    for (Standard_Integer iy = y0; iy <= y1; ++iy)
    {
      const Standard_Real y = iy * m_Step;
      Handle(Graphic3d_ArrayOfSegments)& target = isMajor(iy) ? segMajor : segMinor;
      target->AddVertex((Standard_ShortReal)m_XMin, (Standard_ShortReal)y, 0.0f);
      target->AddVertex((Standard_ShortReal)m_XMax, (Standard_ShortReal)y, 0.0f);
      target->AddEdge(2);
    }

    if (segMinor->VertexNumber() > 0)
    {
      Handle(Graphic3d_Group) gMinor = thePrs->NewGroup();
      gMinor->SetGroupPrimitivesAspect(new Graphic3d_AspectLine3d(kGridMinorColor(), Aspect_TOL_SOLID, 2.0f));
      gMinor->AddPrimitiveArray(segMinor);
    }
    if (segMajor->VertexNumber() > 0)
    {
      Handle(Graphic3d_Group) gMajor = thePrs->NewGroup();
      gMajor->SetGroupPrimitivesAspect(new Graphic3d_AspectLine3d(kGridMajorColor(), Aspect_TOL_SOLID, 3.0f));
      gMajor->AddPrimitiveArray(segMajor);
    }
  }

  // Axes: separate groups and arrays for X and Y with distinct colors
  {
    // X axis
    Handle(Graphic3d_ArrayOfSegments) aAxisX = new Graphic3d_ArrayOfSegments(2, 1);
    aAxisX->AddVertex((Standard_ShortReal)m_XMin, 0.0f, 0.0f);
    aAxisX->AddVertex((Standard_ShortReal)m_XMax, 0.0f, 0.0f); aAxisX->AddEdge(2);
    Handle(Graphic3d_Group) gX = thePrs->NewGroup();
    gX->SetGroupPrimitivesAspect(new Graphic3d_AspectLine3d(kAxesXColor(), Aspect_TOL_SOLID, 2.0f));
    gX->AddPrimitiveArray(aAxisX);

    // Y axis
    Handle(Graphic3d_ArrayOfSegments) aAxisY = new Graphic3d_ArrayOfSegments(2, 1);
    aAxisY->AddVertex(0.0f, (Standard_ShortReal)m_YMin, 0.0f);
    aAxisY->AddVertex(0.0f, (Standard_ShortReal)m_YMax, 0.0f); aAxisY->AddEdge(2);
    Handle(Graphic3d_Group) gY = thePrs->NewGroup();
    gY->SetGroupPrimitivesAspect(new Graphic3d_AspectLine3d(kAxesYColor(), Aspect_TOL_SOLID, 2.0f));
    gY->AddPrimitiveArray(aAxisY);
  }

  // Origin marker: small cross sized relative to step
  {
    const Standard_Real s = Max(0.25 * m_Step, 1.0e-3);
    Handle(Graphic3d_ArrayOfSegments) aCross = new Graphic3d_ArrayOfSegments(4, 2);
    aCross->AddVertex((Standard_ShortReal)(-s), 0.0f, 0.0f);
    aCross->AddVertex((Standard_ShortReal)(+s), 0.0f, 0.0f); aCross->AddEdge(2);
    aCross->AddVertex(0.0f, (Standard_ShortReal)(-s), 0.0f);
    aCross->AddVertex(0.0f, (Standard_ShortReal)(+s), 0.0f); aCross->AddEdge(2);
    Handle(Graphic3d_Group) gO = thePrs->NewGroup();
    gO->SetGroupPrimitivesAspect(new Graphic3d_AspectLine3d(kOriginColor(), Aspect_TOL_SOLID, 2.0f));
    gO->AddPrimitiveArray(aCross);
  }
}
