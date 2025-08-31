// Copyright (c) 2023 Kirill Gavrilov

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
// Forward declares for guides/trihedron
class AIS_Line;
class AIS_Trihedron;
class AIS_Shape;
class Geom_Axis2Placement;

class AIS_ViewCube;

//! OpenGL Qt widget holding OCCT 3D View.
//!
//! OCCT 3D Viewer will reuse OpenGL context created by QOpenGLWidget;
//! widgets on top will be blended by Qt naturally.
//!
//! Inheritance from AIS_ViewController is used to translate
//! user input events (mouse, keyboard, window resize, etc.)
//! to 3D Viewer (panning, rotation, zooming, etc.).
class OcctQOpenGLWidgetViewer : public QOpenGLWidget, public AIS_ViewController
{
  Q_OBJECT
public:
  //! Main constructor.
  OcctQOpenGLWidgetViewer(QWidget* theParent = nullptr);

  //! Destructor.
  virtual ~OcctQOpenGLWidgetViewer();

  //! Return Viewer.
  const Handle(V3d_Viewer)& Viewer() const { return myViewer; }

  //! Return View.
  const Handle(V3d_View)& View() const { return myView; }

  //! Return AIS context.
  const Handle(AIS_InteractiveContext)& Context() const { return myContext; }

  //! Return OpenGL info.
  const QString& getGlInfo() const { return myGlInfo; }

  //! Minimal widget size.
  virtual QSize minimumSizeHint() const override { return QSize(200, 200); }

  //! Default widget size.
  virtual QSize sizeHint() const override { return QSize(720, 480); }

public:
  //! Add body (TopoDS_Shape) into the scene and track it as a "body".
  //! Returns handle to AIS_Shape for optional external use.
  Handle(AIS_Shape) addBody(const TopoDS_Shape& theShape,
                            AIS_DisplayMode    theDispMode = AIS_Shaded,
                            Standard_Integer   theDispPriority = 0,
                            bool               theToUpdate = false);

  //! Show/hide all tracked bodies (AIS_Shape), keeping guides visible.
  void setBodiesVisible(bool theVisible);

  //! Toggle bodies visibility, returns new state.
  bool toggleBodiesVisible();

  //! Handle subview focus change.
  virtual void OnSubviewChanged(const Handle(AIS_InteractiveContext)&,
                                const Handle(V3d_View)&,
                                const Handle(V3d_View)& theNewView) override;

protected: // OpenGL events
  virtual void initializeGL() override;
  virtual void paintGL() override;
  // virtual void resizeGL(int , int ) override;

protected: // user input events
  virtual void closeEvent(QCloseEvent* theEvent) override;
  virtual void keyPressEvent(QKeyEvent* theEvent) override;
  virtual void mousePressEvent(QMouseEvent* theEvent) override;
  virtual void mouseReleaseEvent(QMouseEvent* theEvent) override;
  virtual void mouseMoveEvent(QMouseEvent* theEvent) override;
  virtual void wheelEvent(QWheelEvent* theEvent) override;

private:
  //! Dump OpenGL info.
  void dumpGlInfo(bool theIsBasic, bool theToPrint);

  //! Request widget paintGL() event.
  void updateView();

  //! Handle view redraw.
  virtual void handleViewRedraw(const Handle(AIS_InteractiveContext)& theCtx, const Handle(V3d_View)& theView) override;

private:
  //! Recompute grid step to keep ~constant on-screen spacing.
  void updateGridStepForView(const Handle(V3d_View)& theView);

  //! Compute intersection of screen ray with plane Z=0 at given pixel.
  //! Returns false if ray is parallel to plane.
  bool rayHitZ0(const Handle(V3d_View)& theView,
                int                     thePx,
                int                     thePy,
                gp_Pnt&                 theHit) const;

  Handle(V3d_Viewer)             myViewer;
  Handle(V3d_View)               myView;
  Handle(AIS_InteractiveContext) myContext;
  Handle(AIS_ViewCube)           myViewCube;

  // Guides and origin icon
  Handle(AIS_Line)               myAxisX;
  Handle(AIS_Line)               myAxisY;
  Handle(AIS_Trihedron)          myOriginTrihedron;
  Handle(Geom_Axis2Placement)    myOriginPlacement;

  // Bodies tracking and visibility
  NCollection_Sequence<Handle(AIS_Shape)> myBodies;
  bool                                    myBodiesVisible = true;

  Handle(V3d_View) myFocusView;

  QString myGlInfo;
  bool    myIsCoreProfile = true;

  // Cached current grid step in model units
  double  myGridStep = 10.0;
};

#endif // _OcctQOpenGLWidgetViewer_HeaderFile
