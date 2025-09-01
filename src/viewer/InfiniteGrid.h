// Adaptive infinite grid displayed in the XY plane (Z=0)
#ifndef _InfiniteGrid_HeaderFile
#define _InfiniteGrid_HeaderFile

#include <AIS_InteractiveObject.hxx>
#include <V3d_View.hxx>

// Draws view-filling grid segments with adaptive step and extents.
// - Call updateFromView() when the camera moves to recompute step/extents.
class InfiniteGrid : public AIS_InteractiveObject
{
  DEFINE_STANDARD_RTTIEXT(InfiniteGrid, AIS_InteractiveObject)
public:
  InfiniteGrid() = default;

  // Update grid step and extents from the current view.
  void updateFromView(const Handle(V3d_View)& theView);

  // Lightweight test hook: update using provided viewport size and world-per-20px sample.
  void updateFromViewportSample(Standard_Integer theVpW, Standard_Integer theVpH, Standard_Real theWorldPer20px);

  // Introspection for tests
  Standard_Real step() const { return m_Step; }
  Standard_Real xmin() const { return m_XMin; }
  Standard_Real xmax() const { return m_XMax; }
  Standard_Real ymin() const { return m_YMin; }
  Standard_Real ymax() const { return m_YMax; }

public: // AIS_InteractiveObject
  virtual void Compute(const Handle(PrsMgr_PresentationManager3d)& thePrsMgr,
                       const Handle(Prs3d_Presentation)&            thePrs,
                       const Standard_Integer                       theMode) override;
  virtual void ComputeSelection(const Handle(SelectMgr_Selection)&, const Standard_Integer) override {}

private:
  static Standard_Boolean rayHitZ0(const Handle(V3d_View)& theView, int thePx, int thePy, gp_Pnt& theHit);
  void                    computeStepFromWorldPer20px(Standard_Real theVpW, Standard_Real theVpH, Standard_Real theWorldPer20px);
  void                    computeExtentsFromView(const Handle(V3d_View)& theView, Standard_Integer theVpW, Standard_Integer theVpH);

private:
  // Minor grid step (base). Keep constant 1.0; major lines are every 10th.
  Standard_Real m_Step = 1.0;
  Standard_Real m_XMin = -100.0, m_XMax = 100.0;
  Standard_Real m_YMin = -100.0, m_YMax = 100.0;
  Standard_Boolean m_Initialized = Standard_False;
};

DEFINE_STANDARD_HANDLE(InfiniteGrid, AIS_InteractiveObject)

#endif
