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

class OcctQOpenGLWidgetViewer : public QOpenGLWidget, public AIS_ViewController
{
  Q_OBJECT
public:
  OcctQOpenGLWidgetViewer(QWidget* theParent = nullptr);
  virtual ~OcctQOpenGLWidgetViewer();

  const Handle(V3d_Viewer)& Viewer() const { return m_viewer; }
  const Handle(V3d_View)& View() const { return m_view; }
  const Handle(AIS_InteractiveContext)& Context() const { return m_context; }
  const QString& getGlInfo() const { return m_glInfo; }

  virtual QSize minimumSizeHint() const override { return QSize(200, 200); }
  virtual QSize sizeHint() const override { return QSize(720, 480); }

public:
  virtual void OnSubviewChanged(const Handle(AIS_InteractiveContext)&,
                                const Handle(V3d_View)&,
                                const Handle(V3d_View)& theNewView) override;

protected:
  virtual void initializeGL() override;
  virtual void paintGL() override;

protected:
  virtual void closeEvent(QCloseEvent* theEvent) override;
  virtual void keyPressEvent(QKeyEvent* theEvent) override;
  virtual void mousePressEvent(QMouseEvent* theEvent) override;
  virtual void mouseReleaseEvent(QMouseEvent* theEvent) override;
  virtual void mouseMoveEvent(QMouseEvent* theEvent) override;
  virtual void wheelEvent(QWheelEvent* theEvent) override;

public: // bodies management
  Handle(AIS_Shape) addBody(const TopoDS_Shape& theShape,
                            AIS_DisplayMode    theDispMode = AIS_Shaded,
                            Standard_Integer   theDispPriority = 0,
                            bool               theToUpdate = false);
  void clearBodies(bool theToUpdate = true);
  // Alias for clarity: add a shape into AIS context
  Handle(AIS_Shape) addShape(const TopoDS_Shape& theShape,
                             AIS_DisplayMode    theDispMode = AIS_Shaded,
                             Standard_Integer   theDispPriority = 0,
                             bool               theToUpdate = false)
  { return addBody(theShape, theDispMode, theDispPriority, theToUpdate); }
  Handle(AIS_Shape) selectedShape() const;
  Handle(AIS_Shape) detectedShape() const;
  // visibility toggling removed; viewer keeps all displayed bodies

private:
  void dumpGlInfo(bool theIsBasic, bool theToPrint);
  void updateView();
  virtual void handleViewRedraw(const Handle(AIS_InteractiveContext)& theCtx,
                                const Handle(V3d_View)&               theView) override;

  void updateGridStepForView(const Handle(V3d_View)& theView);
  bool rayHitZ0(const Handle(V3d_View)& theView, int thePx, int thePy, gp_Pnt& theHit) const;

private:
  Handle(V3d_Viewer)             m_viewer;
  Handle(V3d_View)               m_view;
  Handle(AIS_InteractiveContext) m_context;
  Handle(AIS_ViewCube)           m_viewCube;
  Handle(V3d_View)               m_focusView;
  QString                        m_glInfo;
  bool                           m_isCoreProfile = true;
  double                         m_gridStep = 10.0;

  Handle(AIS_Line)               m_axisX;
  Handle(AIS_Line)               m_axisY;
  Handle(AIS_Trihedron)          m_originTrihedron;
  Handle(Geom_Axis2Placement)    m_originPlacement;
  NCollection_Sequence<Handle(AIS_Shape)> m_bodies;

  // no deletion-specific state
};

#endif
