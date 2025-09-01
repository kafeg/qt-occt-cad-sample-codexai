// Moved viewer widget (rendering + input translation), reusable across modes
#ifndef _OcctQOpenGLWidgetViewer_HeaderFile
#define _OcctQOpenGLWidgetViewer_HeaderFile

#include <Standard_WarningsDisable.hxx>
#include <QOpenGLWidget>
#include <Standard_WarningsRestore.hxx>

#include <AIS_InteractiveContext.hxx>
#include <AIS_ViewController.hxx>
#include <V3d_View.hxx>
#include <NCollection_Sequence.hxx>
#include <TopoDS_Shape.hxx>

class AIS_ViewCube;
class AIS_Line;
class AIS_Trihedron;
class AIS_Shape;
class Geom_Axis2Placement;
class Sketch; // forward decl (from src/sketch)

// Reusable OCCT viewer widget (QOpenGLWidget + AIS_ViewController glue)
// - Owns V3d_Viewer/View and AIS_InteractiveContext
// - Renders, handles input, selection, view cube and axes
class OcctQOpenGLWidgetViewer : public QOpenGLWidget, public AIS_ViewController
{
  Q_OBJECT
public:
  OcctQOpenGLWidgetViewer(QWidget* theParent = nullptr);
  virtual ~OcctQOpenGLWidgetViewer();

  const Handle(V3d_Viewer)& Viewer() const { return m_viewer; }              // OCCT viewer
  const Handle(V3d_View)& View() const { return m_view; }                    // active view
  const Handle(AIS_InteractiveContext)& Context() const { return m_context; }// AIS context
  const QString& getGlInfo() const { return m_glInfo; }                      // aggregated GL info

  virtual QSize minimumSizeHint() const override { return QSize(200, 200); }
  virtual QSize sizeHint() const override { return QSize(720, 480); }

public:
  // Reset camera: look at origin with a fixed isometric projection and
  // place eye at (0,0,distance) regardless of previous state (default 5.0)
  void resetViewToOrigin(Standard_Real distance = 5.0);

public:
  virtual void OnSubviewChanged(const Handle(AIS_InteractiveContext)&,
                                const Handle(V3d_View)&,
                                const Handle(V3d_View)& theNewView) override; // track focus subview

protected:
  virtual void initializeGL() override; // bind OCCT view to Qt surface
  virtual void paintGL() override;      // render frame

protected:
  virtual void closeEvent(QCloseEvent* theEvent) override;       // simple accept
  virtual void keyPressEvent(QKeyEvent* theEvent) override;       // fit-all helper
  virtual void mousePressEvent(QMouseEvent* theEvent) override;   // delegate to AIS_ViewController
  virtual void mouseReleaseEvent(QMouseEvent* theEvent) override; // single-select on click
  virtual void mouseMoveEvent(QMouseEvent* theEvent) override;    // orbit/pan/rotate
  virtual void wheelEvent(QWheelEvent* theEvent) override;        // zoom + subviews

public: // bodies management
  Handle(AIS_Shape) addBody(const TopoDS_Shape& theShape, // display a shape and keep handle
                            AIS_DisplayMode    theDispMode = AIS_Shaded,
                            Standard_Integer   theDispPriority = 0,
                            bool               theToUpdate = false);
  void clearBodies(bool theToUpdate = true); // erase all tracked bodies
  // Alias for clarity: add a shape into AIS context
  Handle(AIS_Shape) addShape(const TopoDS_Shape& theShape,
                             AIS_DisplayMode    theDispMode = AIS_Shaded,
                             Standard_Integer   theDispPriority = 0,
                             bool               theToUpdate = false)
  { return addBody(theShape, theDispMode, theDispPriority, theToUpdate); }
  Handle(AIS_Shape) selectedShape() const;
  Handle(AIS_Shape) detectedShape() const;
  // visibility toggling removed; viewer keeps all displayed bodies

public: // sketches management
  // Display a sketch as a colored wire compound and keep handle for cleanup
  Handle(AIS_Shape) addSketch(const std::shared_ptr<Sketch>& sketch);
  // Remove all displayed sketches
  void clearSketches(bool theToUpdate = true);
  // Introspection helper for tests
  int sketchCount() const { return m_sketches.Size(); }

signals:
  void selectionChanged();

private:
  void dumpGlInfo(bool theIsBasic, bool theToPrint); // collect GL info string
  void updateView();                                  // schedule repaint
  virtual void handleViewRedraw(const Handle(AIS_InteractiveContext)& theCtx,
                                const Handle(V3d_View)&               theView) override;

  bool rayHitZ0(const Handle(V3d_View)& theView, int thePx, int thePy, gp_Pnt& theHit) const; // project to Z=0

private:
  Handle(V3d_Viewer)             m_viewer;           // core OCCT viewer
  Handle(V3d_View)               m_view;             // main view
  Handle(AIS_InteractiveContext) m_context;          // AIS context for display/selection
  Handle(AIS_ViewCube)           m_viewCube;         // persistent view cube
  Handle(V3d_View)               m_focusView;        // current focus subview (if any)
  QString                        m_glInfo;           // GL diagnostics info
  bool                           m_isCoreProfile = true; // prefer core GL profile
  Handle(AIS_InteractiveObject)  m_grid;             // custom infinite grid

  Handle(AIS_Line)               m_axisX;            // X axis guide
  Handle(AIS_Line)               m_axisY;            // Y axis guide
  Handle(AIS_Trihedron)          m_originTrihedron;  // origin trihedron
  Handle(Geom_Axis2Placement)    m_originPlacement;  // origin placement
  NCollection_Sequence<Handle(AIS_Shape)> m_bodies;  // tracked displayed bodies
  NCollection_Sequence<Handle(AIS_Shape)> m_sketches; // tracked displayed sketches

  // no deletion-specific state
};

#endif
