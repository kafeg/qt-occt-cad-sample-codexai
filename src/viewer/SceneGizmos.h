// Axes + Trihedron overlay manager
#ifndef _SceneGizmos_HeaderFile
#define _SceneGizmos_HeaderFile

#include <Standard_Type.hxx>
#include <Standard_Boolean.hxx>
#include <Standard_Real.hxx>
#include <memory>

class Datum;
class AIS_InteractiveContext;
class AIS_InteractiveObject;
class AIS_Trihedron;
class AIS_Shape;
class AIS_TexturedShape;

class SceneGizmos
{
public:
  SceneGizmos() = default;

  // Idempotent install/update driven by Datum flags; creates, updates or hides items as needed.
  void install(const Handle(AIS_InteractiveContext)& ctx,
               const std::shared_ptr<Datum>&          datum,
               Standard_Boolean                        topmostOverlay = Standard_True);

  // Re-display already created items without rebuilding (used after FitAll etc.).
  void reinstall(const Handle(AIS_InteractiveContext)& ctx);

  // Erase from context, keep handles alive for quick reinstall.
  void erase(const Handle(AIS_InteractiveContext)& ctx);

  Handle(AIS_InteractiveObject) bgAxisX() const { return m_bgAxisX; }
  Handle(AIS_InteractiveObject) bgAxisY() const { return m_bgAxisY; }
  Handle(AIS_Trihedron)         trihedron() const { return m_trihedron; }

  // Update background axis extents (if enabled); hides them when disabled.
  void setAxisExtents(const Handle(AIS_InteractiveContext)& ctx, Standard_Real halfX, Standard_Real halfY);

private:
  // Cached handles
  Handle(AIS_InteractiveObject) m_bgAxisX;
  Handle(AIS_InteractiveObject) m_bgAxisY;
  Handle(AIS_Trihedron)         m_trihedron;
  Handle(AIS_Shape)             m_planeYZ;
  Handle(AIS_Shape)             m_planeXZ;
  Handle(AIS_Shape)             m_planeXY;
  Handle(AIS_Shape)             m_originMark;
  Handle(AIS_TexturedShape)     m_bgOriginSprite;
  bool                          m_showBgAxes = false;
};

#endif
