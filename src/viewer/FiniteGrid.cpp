#include "FiniteGrid.h"

#include <Prs3d_Root.hxx>
#include <Prs3d_Presentation.hxx>
#include <Prs3d_Text.hxx>
#include <Prs3d_TextAspect.hxx>
#include <TCollection_ExtendedString.hxx>
#include <Graphic3d_ArrayOfSegments.hxx>
#include <Graphic3d_AspectLine3d.hxx>
#include <Aspect_NeutralWindow.hxx>
#include <Quantity_Color.hxx>
#include <gp_Pnt.hxx>

IMPLEMENT_STANDARD_RTTIEXT(FiniteGrid, AIS_InteractiveObject)

namespace {
// Softer, less intrusive tones
static inline Quantity_Color kGridMinorColor() { return Quantity_Color(Quantity_NOC_GRAY90); }
static inline Quantity_Color kGridMajorColor() { return Quantity_Color(Quantity_NOC_GRAY70); }
}

void FiniteGrid::updateFromView(const Handle(V3d_View)& theView)
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

  // Sample world distance corresponding to deltaX pixels using center and near-center (robust to precision)
  const int cx = vpW / 2; const int cy = vpH / 2; gp_Pnt p0, p1;
  int sampleDx = std::max(8, vpW / 20); // adaptive pixel baseline to reduce numeric issues at extreme zoom
  if (!rayHitZ0(theView, cx, cy, p0) || !rayHitZ0(theView, cx + sampleDx, cy, p1))
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
    const Standard_Real worldPerPx = p0.Distance(p1) / (Standard_Real)sampleDx;
    const Standard_Real worldPerTarget = worldPerPx * m_TargetPixels;
    // Reuse function name but pass world distance corresponding to m_TargetPixels
    computeStepFromWorldPer20px(vpW, vpH, worldPerTarget);
  }
  computeExtentsFromView(theView, vpW, vpH);
  clampExtentsToFinite();
  SetToUpdate();
}

void FiniteGrid::updateFromViewportSample(Standard_Integer theVpW, Standard_Integer theVpH, Standard_Real theWorldPer20px)
{
  // Convert legacy 20px sample into target pixel sample
  const Standard_Real worldPerPx = theWorldPer20px / 20.0;
  computeStepFromWorldPer20px(theVpW, theVpH, worldPerPx * m_TargetPixels);
  // Extents depend on view framing; without a view, derive generous extents covering viewport
  // Assuming a typical perspective, span ~12 steps around origin as a safe default
  const Standard_Real marginSteps = 12.0;
  const Standard_Real span = m_Step * marginSteps;
  m_XMin = -span; m_XMax = span;
  m_YMin = -span; m_YMax = span;
  m_Initialized = Standard_True;
  SetToUpdate();
}

void FiniteGrid::computeStepFromWorldPer20px(Standard_Integer, Standard_Integer, Standard_Real theWorldPerTargetPx)
{
  // Derive minor grid step from a 20px sample using a 1–2–5*10^n series.
  // Aim ~m_TargetPixels per cell: snap to nearest {1,2,5,10}*10^n with hysteresis.
  if (!(theWorldPerTargetPx > 0.0) || !std::isfinite((double)theWorldPerTargetPx))
  {
    if (m_Step <= 0.0) m_Step = 1.0;
    return;
  }

  const Standard_Real x = theWorldPerTargetPx;
  const Standard_Real n = Floor(Log10(x));
  const Standard_Real base = Pow(10.0, n);
  const Standard_Real r = x / base; // in [1,10)

  Standard_Real mantissa = 1.0;
  if (r < 1.5)      mantissa = 1.0;
  else if (r < 3.0) mantissa = 2.0;
  else if (r < 7.0) mantissa = 5.0;
  else              mantissa = 10.0;
  const Standard_Real candidate = mantissa * base;

  // Hysteresis to reduce flicker near thresholds
  if (m_Initialized)
  {
    const Standard_Real lo = m_Step * (1.0 - m_Hysteresis);
    const Standard_Real hi = m_Step * (1.0 + m_Hysteresis);
    if (candidate > lo && candidate < hi)
    {
      // keep previous step
    }
    else
    {
      m_Step = candidate;
    }
  }
  else
  {
    m_Step = candidate;
  }

  if (!(m_Step > 0.0) || !std::isfinite((double)m_Step))
  {
    m_Step = 1.0;
  }
}

void FiniteGrid::computeExtentsFromView(const Handle(V3d_View)& theView, Standard_Integer theVpW, Standard_Integer theVpH)
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
  // Add a margin beyond viewport; clamp later to finite bounds
  const Standard_Real m = 1.5 * m_Step;
  m_XMin = floor((xmin - m) / m_Step) * m_Step;
  m_XMax = ceil ((xmax + m) / m_Step) * m_Step;
  m_YMin = floor((ymin - m) / m_Step) * m_Step;
  m_YMax = ceil ((ymax + m) / m_Step) * m_Step;
  m_Initialized = Standard_True;
}

void FiniteGrid::clampExtentsToFinite()
{
  const Standard_Real xMinFinite = -m_HalfSizeX;
  const Standard_Real xMaxFinite =  m_HalfSizeX;
  const Standard_Real yMinFinite = -m_HalfSizeY;
  const Standard_Real yMaxFinite =  m_HalfSizeY;

  m_XMin = Max(m_XMin, xMinFinite);
  m_XMax = Min(m_XMax, xMaxFinite);
  m_YMin = Max(m_YMin, yMinFinite);
  m_YMax = Min(m_YMax, yMaxFinite);
}

Standard_Boolean FiniteGrid::rayHitZ0(const Handle(V3d_View)& theView, int thePx, int thePy, gp_Pnt& theHit)
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

void FiniteGrid::Compute(const Handle(PrsMgr_PresentationManager)&,
                           const Handle(Prs3d_Presentation)&         thePrs,
                           const Standard_Integer)
{
  thePrs->Clear();
  if (!m_Initialized || m_Step <= 0.0) return;

  // Apply subtle transparency and ensure depth test is enabled (world anchored)
  Handle(Prs3d_Drawer) aDr = Attributes(); if (aDr.IsNull()) aDr = new Prs3d_Drawer();
  aDr->SetTransparency(0.80f); // 0=opaque, 1=fully transparent
  aDr->SetZLayer(Graphic3d_ZLayerId_Default);
  SetAttributes(aDr);

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
    // Unindexed arrays: pairs of vertices define segments implicitly
    Handle(Graphic3d_ArrayOfSegments) segMinor = new Graphic3d_ArrayOfSegments(2 * gridSegs);
    Handle(Graphic3d_ArrayOfSegments) segMajor = new Graphic3d_ArrayOfSegments(2 * gridSegs / 10 + 4);

    auto isMajor = [](Standard_Integer i) {
      Standard_Integer m = i % 10; if (m < 0) m += 10; return m == 0; };

    // Depth bias to avoid z-fighting with axes/origin (push grid slightly "behind")
    const Standard_Real zBias = -1.0e-3 * Max(m_Step, 1.0);

    // Vertical lines (constant X)
    for (Standard_Integer ix = x0; ix <= x1; ++ix)
    {
      const Standard_Real x = ix * m_Step;
      Handle(Graphic3d_ArrayOfSegments)& target = isMajor(ix) ? segMajor : segMinor;
      target->AddVertex((Standard_ShortReal)x, (Standard_ShortReal)m_YMin, (Standard_ShortReal)zBias);
      target->AddVertex((Standard_ShortReal)x, (Standard_ShortReal)m_YMax, (Standard_ShortReal)zBias);
    }
    // Horizontal lines (constant Y)
    for (Standard_Integer iy = y0; iy <= y1; ++iy)
    {
      const Standard_Real y = iy * m_Step;
      Handle(Graphic3d_ArrayOfSegments)& target = isMajor(iy) ? segMajor : segMinor;
      target->AddVertex((Standard_ShortReal)m_XMin, (Standard_ShortReal)y, (Standard_ShortReal)zBias);
      target->AddVertex((Standard_ShortReal)m_XMax, (Standard_ShortReal)y, (Standard_ShortReal)zBias);
    }

    if (segMinor->VertexNumber() > 0)
    {
      Handle(Graphic3d_Group) gMinor = thePrs->NewGroup();
      gMinor->SetGroupPrimitivesAspect(new Graphic3d_AspectLine3d(kGridMinorColor(), Aspect_TOL_DASH, 1.0f));
      gMinor->AddPrimitiveArray(segMinor);
    }
    if (segMajor->VertexNumber() > 0)
    {
      Handle(Graphic3d_Group) gMajor = thePrs->NewGroup();
      gMajor->SetGroupPrimitivesAspect(new Graphic3d_AspectLine3d(kGridMajorColor(), Aspect_TOL_SOLID, 1.5f));
      gMajor->AddPrimitiveArray(segMajor);
    }
  }
  // Border rectangle and simple edge ticks (no numbers)
  // Border rectangle
  Handle(Graphic3d_ArrayOfSegments) border = new Graphic3d_ArrayOfSegments(8);
  const Standard_Real zBiasBorder = -1.0e-3 * Max(m_Step, 1.0);
  const Standard_Real x0b = -m_HalfSizeX, x1b =  m_HalfSizeX;
  const Standard_Real y0b = -m_HalfSizeY, y1b =  m_HalfSizeY;
  border->AddVertex((Standard_ShortReal)x0b, (Standard_ShortReal)y0b, (Standard_ShortReal)zBiasBorder);
  border->AddVertex((Standard_ShortReal)x1b, (Standard_ShortReal)y0b, (Standard_ShortReal)zBiasBorder);
  border->AddVertex((Standard_ShortReal)x1b, (Standard_ShortReal)y0b, (Standard_ShortReal)zBiasBorder);
  border->AddVertex((Standard_ShortReal)x1b, (Standard_ShortReal)y1b, (Standard_ShortReal)zBiasBorder);
  border->AddVertex((Standard_ShortReal)x1b, (Standard_ShortReal)y1b, (Standard_ShortReal)zBiasBorder);
  border->AddVertex((Standard_ShortReal)x0b, (Standard_ShortReal)y1b, (Standard_ShortReal)zBiasBorder);
  border->AddVertex((Standard_ShortReal)x0b, (Standard_ShortReal)y1b, (Standard_ShortReal)zBiasBorder);
  border->AddVertex((Standard_ShortReal)x0b, (Standard_ShortReal)y0b, (Standard_ShortReal)zBiasBorder);
  Handle(Graphic3d_Group) gBorder = thePrs->NewGroup();
  gBorder->SetGroupPrimitivesAspect(new Graphic3d_AspectLine3d(Quantity_Color(Quantity_NOC_GRAY60), Aspect_TOL_SOLID, 2.0f));
  gBorder->AddPrimitiveArray(border);

  // Draw simple major ticks every 10th step to indicate scale (ensure visible over grid)
  {
    const Standard_Integer ix0 = (Standard_Integer)Ceiling(-m_HalfSizeX / m_Step);
    const Standard_Integer ix1 = (Standard_Integer)Floor  ( m_HalfSizeX / m_Step);
    const Standard_Integer iy0 = (Standard_Integer)Ceiling(-m_HalfSizeY / m_Step);
    const Standard_Integer iy1 = (Standard_Integer)Floor  ( m_HalfSizeY / m_Step);
    auto isMajor = [](Standard_Integer i) { Standard_Integer m = i % 10; if (m < 0) m += 10; return m == 0; };
    const Standard_Real zBiasTicks = -1.5e-3 * Max(m_Step, 1.0);
    const Standard_Real tickLen = 0.50 * m_Step;
    Handle(Graphic3d_ArrayOfSegments) ticks = new Graphic3d_ArrayOfSegments(4 * ((ix1 - ix0 + 1) + (iy1 - iy0 + 1)));
    for (Standard_Integer ix = ix0; ix <= ix1; ++ix)
    {
      if (!isMajor(ix)) continue;
      const Standard_Real x = ix * m_Step;
      // bottom
      ticks->AddVertex((Standard_ShortReal)x, (Standard_ShortReal)(-m_HalfSizeY), (Standard_ShortReal)zBiasTicks);
      ticks->AddVertex((Standard_ShortReal)x, (Standard_ShortReal)(-m_HalfSizeY + tickLen), (Standard_ShortReal)zBiasTicks);
      // top
      ticks->AddVertex((Standard_ShortReal)x, (Standard_ShortReal)( m_HalfSizeY), (Standard_ShortReal)zBiasTicks);
      ticks->AddVertex((Standard_ShortReal)x, (Standard_ShortReal)( m_HalfSizeY - tickLen), (Standard_ShortReal)zBiasTicks);
    }
    for (Standard_Integer iy = iy0; iy <= iy1; ++iy)
    {
      if (!isMajor(iy)) continue;
      const Standard_Real y = iy * m_Step;
      // left
      ticks->AddVertex((Standard_ShortReal)(-m_HalfSizeX), (Standard_ShortReal)y, (Standard_ShortReal)zBiasTicks);
      ticks->AddVertex((Standard_ShortReal)(-m_HalfSizeX + tickLen), (Standard_ShortReal)y, (Standard_ShortReal)zBiasTicks);
      // right
      ticks->AddVertex((Standard_ShortReal)( m_HalfSizeX), (Standard_ShortReal)y, (Standard_ShortReal)zBiasTicks);
      ticks->AddVertex((Standard_ShortReal)( m_HalfSizeX - tickLen), (Standard_ShortReal)y, (Standard_ShortReal)zBiasTicks);
    }
    Handle(Graphic3d_Group) gTicks = thePrs->NewGroup();
    gTicks->SetGroupPrimitivesAspect(new Graphic3d_AspectLine3d(Quantity_Color(Quantity_NOC_GRAY50), Aspect_TOL_SOLID, 1.0f));
    gTicks->AddPrimitiveArray(ticks);
  }
  // Axes/origin are drawn separately in viewer; grid renders only grid lines
}
// Edge scale/labels removed
