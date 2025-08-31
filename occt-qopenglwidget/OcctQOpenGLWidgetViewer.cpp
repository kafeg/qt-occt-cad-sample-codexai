// Copyright (c) 2023 Kirill Gavrilov

#ifdef _WIN32
  // should be included before other headers to avoid missing definitions
  #include <windows.h>
#endif
#include <OpenGl_Context.hxx>

#include "OcctQOpenGLWidgetViewer.h"

#include "OcctGlTools.h"
#include "OcctQtTools.h"

#include <Standard_WarningsDisable.hxx>
#include <QApplication>
#include <QMessageBox>
#include <QMouseEvent>
#include <Standard_WarningsRestore.hxx>

#include <AIS_Shape.hxx>
#include <AIS_ViewCube.hxx>
#include <AIS_Line.hxx>
#include <AIS_Trihedron.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <Aspect_NeutralWindow.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <Quantity_Color.hxx>
#include <Message.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <Standard_Version.hxx>
#include <V3d_RectangularGrid.hxx>
#include <OpenGl_FrameBuffer.hxx>
#include <Geom_Line.hxx>
#include <Geom_Axis2Placement.hxx>
#include <gp_Ax2.hxx>
#include <Prs3d_Drawer.hxx>
#include <Prs3d_LineAspect.hxx>
#include <Prs3d_DatumAspect.hxx>
#include <gp_Pnt.hxx>
#include <cmath>

//! OpenGL FBO subclass for wrapping FBO created by Qt using GL_RGBA8
//! texture format instead of GL_SRGB8_ALPHA8.
//! This FBO is set to OpenGl_Context::SetDefaultFrameBuffer() as a final target.
//! Subclass calls OpenGl_Context::SetFrameBufferSRGB() with sRGB=false flag,
//! which asks OCCT to disable GL_FRAMEBUFFER_SRGB and apply sRGB gamma correction manually.
class OcctQtFrameBuffer : public OpenGl_FrameBuffer
{
  DEFINE_STANDARD_RTTI_INLINE(OcctQtFrameBuffer, OpenGl_FrameBuffer)
public:
  //! Empty constructor.
  OcctQtFrameBuffer() {}

  //! Make this FBO active in context.
  virtual void BindBuffer(const Handle(OpenGl_Context)& theGlCtx) override
  {
    OpenGl_FrameBuffer::BindBuffer(theGlCtx);
    theGlCtx->SetFrameBufferSRGB(true, false);
  }

  //! Make this FBO as drawing target in context.
  virtual void BindDrawBuffer(const Handle(OpenGl_Context)& theGlCtx) override
  {
    OpenGl_FrameBuffer::BindDrawBuffer(theGlCtx);
    theGlCtx->SetFrameBufferSRGB(true, false);
  }

  //! Make this FBO as reading source in context.
  virtual void BindReadBuffer(const Handle(OpenGl_Context)& theGlCtx) override
  {
    OpenGl_FrameBuffer::BindReadBuffer(theGlCtx);
  }
};

// ================================================================
// Function : OcctQOpenGLWidgetViewer
// ================================================================
OcctQOpenGLWidgetViewer::OcctQOpenGLWidgetViewer(QWidget* theParent)
    : QOpenGLWidget(theParent)
{
  Handle(Aspect_DisplayConnection) aDisp   = new Aspect_DisplayConnection();
  Handle(OpenGl_GraphicDriver)     aDriver = new OpenGl_GraphicDriver(aDisp, false);
  // lets QOpenGLWidget to manage buffer swap
  aDriver->ChangeOptions().buffersNoSwap = true;
  // don't write into alpha channel
  aDriver->ChangeOptions().buffersOpaqueAlpha = true;
  // offscreen FBOs should be always used
  aDriver->ChangeOptions().useSystemBuffer = false;

  // create viewer
  myViewer = new V3d_Viewer(aDriver);
  // Fusion-like soft dark grey background gradient
  const Quantity_Color aBgTop(0.24, 0.24, 0.24, Quantity_TOC_sRGB);     // ~#3D3D3D
  const Quantity_Color aBgBottom(0.36, 0.36, 0.36, Quantity_TOC_sRGB);  // ~#5C5C5C
  myViewer->SetDefaultBackgroundColor(aBgTop);
  myViewer->SetDefaultBgGradientColors(aBgTop, aBgBottom, Aspect_GradientFillMethod_Elliptical);
  myViewer->SetDefaultLights();
  myViewer->SetLightOn();
  // Grid: start with 10 mm, dynamic step will adjust on zoom
  myGridStep = 10.0;
  myViewer->SetRectangularGridValues(0.0, 0.0, myGridStep, myGridStep, 0.0);
  myViewer->ActivateGrid(Aspect_GT_Rectangular, Aspect_GDM_Lines);

  // create AIS context
  myContext = new AIS_InteractiveContext(myViewer);

  myViewCube = new AIS_ViewCube();
  myViewCube->SetViewAnimation(myViewAnimation);
  myViewCube->SetFixedAnimationLoop(false);
  myViewCube->SetAutoStartAnimation(true);
  myViewCube->SetSize(60.0);
  myViewCube->SetBoxColor(Quantity_NOC_GRAY70);
  myViewCube->TransformPersistence()->SetCorner2d(Aspect_TOTP_RIGHT_UPPER);
  // move view cube a little further from the screen edges to keep it fully visible
  myViewCube->TransformPersistence()->SetOffset2d(Graphic3d_Vec2i(80, 80));

  // note - window will be created later within initializeGL() callback!
  myView = myViewer->CreateView();
  myView->SetBgGradientColors(aBgTop, aBgBottom, Aspect_GradientFillMethod_Elliptical);
  myView->SetImmediateUpdate(false);
#ifndef __APPLE__
  myView->ChangeRenderingParams().NbMsaaSamples = 4; // warning - affects performance
#endif
  myView->ChangeRenderingParams().ToShowStats = true;
  // enlarge the overlay text for better readability
  myView->ChangeRenderingParams().StatsTextHeight = 24;
  myView->ChangeRenderingParams().CollectedStats =
    (Graphic3d_RenderingParams::PerfCounters)(Graphic3d_RenderingParams::PerfCounters_FrameRate
                                              | Graphic3d_RenderingParams::PerfCounters_Triangles);

  // Qt widget setup
  setMouseTracking(true);
  setBackgroundRole(QPalette::NoRole); // or NoBackground
  setFocusPolicy(Qt::StrongFocus);     // set focus policy to threat QContextMenuEvent from keyboard
  setUpdatesEnabled(true);
  setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);

  // OpenGL setup managed by Qt
  QSurfaceFormat aGlFormat;
  aGlFormat.setDepthBufferSize(24);
  aGlFormat.setStencilBufferSize(8);
  // aGlFormat.setOption (QSurfaceFormat::DebugContext, true);
  aDriver->ChangeOptions().contextDebug = aGlFormat.testOption(QSurfaceFormat::DebugContext);
  // aGlFormat.setOption (QSurfaceFormat::DeprecatedFunctions, true);
  if (myIsCoreProfile)
    aGlFormat.setVersion(4, 5);

  aGlFormat.setProfile(myIsCoreProfile ? QSurfaceFormat::CoreProfile : QSurfaceFormat::CompatibilityProfile);

  // request sRGBColorSpace colorspace to meet OCCT expectations or use OcctQtFrameBuffer fallback.
  /*#if (QT_VERSION_MAJOR > 5) || (QT_VERSION_MAJOR == 5 && QT_VERSION_MINOR >= 10)
    aGlFormat.setColorSpace(QSurfaceFormat::sRGBColorSpace);
    setTextureFormat(GL_SRGB8_ALPHA8);
  #else
    Message::SendWarning("Warning! Qt 5.10+ is required for sRGB setup.\n"
                         "Colors in 3D Viewer might look incorrect (Qt " QT_VERSION_STR " is used).\n");
    aDriver->ChangeOptions().sRGBDisable = true;
  #endif*/

  setFormat(aGlFormat);

#if defined(_WIN32)
  // never use ANGLE on Windows, since OCCT 3D Viewer does not expect this
  QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
  // QCoreApplication::setAttribute (Qt::AA_UseOpenGLES);
#endif
}

// ================================================================
// Function : ~OcctQOpenGLWidgetViewer
// ================================================================
OcctQOpenGLWidgetViewer::~OcctQOpenGLWidgetViewer()
{
  // hold on X11 display connection till making another connection active by glXMakeCurrent()
  // to workaround sudden crash in QOpenGLWidget destructor
  Handle(Aspect_DisplayConnection) aDisp = myViewer->Driver()->GetDisplayConnection();

  // release OCCT viewer
  myContext->RemoveAll(false);
  myContext.Nullify();
  myView->Remove();
  myView.Nullify();
  myViewer.Nullify();

  // make active OpenGL context created by Qt
  makeCurrent();
  aDisp.Nullify();
}

// ================================================================
// Function : dumpGlInfo
// ================================================================
void OcctQOpenGLWidgetViewer::dumpGlInfo(bool theIsBasic, bool theToPrint)
{
  TColStd_IndexedDataMapOfStringString aGlCapsDict;
  myView->DiagnosticInformation(aGlCapsDict,
                                theIsBasic ? Graphic3d_DiagnosticInfo_Basic : Graphic3d_DiagnosticInfo_Complete);
  TCollection_AsciiString anInfo;
  for (TColStd_IndexedDataMapOfStringString::Iterator aValueIter(aGlCapsDict); aValueIter.More(); aValueIter.Next())
  {
    if (!aValueIter.Value().IsEmpty())
    {
      if (!anInfo.IsEmpty())
        anInfo += "\n";

      anInfo += aValueIter.Key() + ": " + aValueIter.Value();
    }
  }

  if (theToPrint)
    Message::SendInfo(anInfo);

  myGlInfo = QString::fromUtf8(anInfo.ToCString());
}

// ================================================================
// Function : initializeGL
// ================================================================
void OcctQOpenGLWidgetViewer::initializeGL()
{
  const QRect           aRect       = rect();
  const qreal           aPixelRatio = devicePixelRatioF();
  const Graphic3d_Vec2i aViewSize((aRect.right() - aRect.left()) * aPixelRatio,
                                  (aRect.bottom() - aRect.top()) * aPixelRatio);

  Aspect_Drawable aNativeWin = (Aspect_Drawable)winId();
#ifdef _WIN32
  HDC  aWglDevCtx = wglGetCurrentDC();
  HWND aWglWin    = WindowFromDC(aWglDevCtx);
  aNativeWin      = (Aspect_Drawable)aWglWin;
#endif

  Handle(OpenGl_Context) aGlCtx = new OpenGl_Context();
  if (!aGlCtx->Init(myIsCoreProfile))
  {
    Message::SendFail() << "Error: OpenGl_Context is unable to wrap OpenGL context";
    QMessageBox::critical(0, "Failure", "OpenGl_Context is unable to wrap OpenGL context");
    QApplication::exit(1);
    return;
  }

  Handle(Aspect_NeutralWindow) aWindow = Handle(Aspect_NeutralWindow)::DownCast(myView->Window());
  if (!aWindow.IsNull())
  {
    aWindow->SetNativeHandle(aNativeWin);
    aWindow->SetSize(aViewSize.x(), aViewSize.y());
    myView->SetWindow(aWindow, aGlCtx->RenderingContext());
    dumpGlInfo(true, true);
  }
  else
  {
    aWindow = new Aspect_NeutralWindow();
    aWindow->SetVirtual(true);
    aWindow->SetNativeHandle(aNativeWin);
    aWindow->SetSize(aViewSize.x(), aViewSize.y());
    myView->SetWindow(aWindow, aGlCtx->RenderingContext());
    dumpGlInfo(true, true);

    myContext->Display(myViewCube, 0, 0, false);
  }

  {
    // dummy body for testing via helper method (also tracked for toggling)
    TopoDS_Shape aBox = BRepPrimAPI_MakeBox(100.0, 50.0, 90.0).Shape();
    addBody(aBox, AIS_Shaded, 0, false);
    // center the view on the displayed shape
    myView->FitAll(0.01, false);
    // adjust grid spacing to initial zoom level
    updateGridStepForView(myView);

    // Add Fusion-like guides and origin icon at world origin (after FitAll to avoid skewing zoom)
    // Use finite segments so they don't explode FitAll; length can be tuned as needed
    const Standard_Real L = 1000.0;
    Handle(Geom_Line) aGeomX = new Geom_Line(gp_Pnt(-L, 0.0, 0.0), gp_Dir(1.0, 0.0, 0.0));
    Handle(Geom_Line) aGeomY = new Geom_Line(gp_Pnt(0.0, -L, 0.0), gp_Dir(0.0, 1.0, 0.0));
    myAxisX = new AIS_Line(aGeomX);
    myAxisY = new AIS_Line(aGeomY);
    myAxisX->SetColor(Quantity_NOC_RED);
    myAxisY->SetColor(Quantity_NOC_GREEN);
    // slightly thicker lines for visibility
    Handle(Prs3d_Drawer) xDr = myAxisX->Attributes();
    if (xDr.IsNull()) xDr = new Prs3d_Drawer();
    xDr->SetLineAspect(new Prs3d_LineAspect(Quantity_NOC_RED, Aspect_TOL_SOLID, 2.0f));
    myAxisX->SetAttributes(xDr);
    Handle(Prs3d_Drawer) yDr = myAxisY->Attributes();
    if (yDr.IsNull()) yDr = new Prs3d_Drawer();
    yDr->SetLineAspect(new Prs3d_LineAspect(Quantity_NOC_GREEN, Aspect_TOL_SOLID, 2.0f));
    myAxisY->SetAttributes(yDr);
    myContext->Display(myAxisX, 0, 1, false);
    myContext->Display(myAxisY, 0, 1, false);

    // Origin trihedron (center icon) at world origin
    myOriginPlacement = new Geom_Axis2Placement(gp_Ax2(gp::Origin(), gp::DZ(), gp::DX()));
    myOriginTrihedron = new AIS_Trihedron(myOriginPlacement);
    // use default colors (X=red, Y=green, Z=blue); make it compact
    Handle(Prs3d_Drawer) trDr = myOriginTrihedron->Attributes();
    if (trDr.IsNull()) trDr = new Prs3d_Drawer();
    trDr->SetDatumAspect(new Prs3d_DatumAspect());
    trDr->DatumAspect()->SetAxisLength(20.0, 20.0, 20.0);
    myOriginTrihedron->SetAttributes(trDr);
    myContext->Display(myOriginTrihedron, 0, 2, false);
  }
}

// ================================================================
// Function : closeEvent
// ================================================================
void OcctQOpenGLWidgetViewer::closeEvent(QCloseEvent* theEvent)
{
  theEvent->accept();
}

// ================================================================
// Function : keyPressEvent
// ================================================================
void OcctQOpenGLWidgetViewer::keyPressEvent(QKeyEvent* theEvent)
{
  if (myView.IsNull())
    return;

  const Aspect_VKey aKey = OcctQtTools::qtKey2VKey(theEvent->key());
  switch (aKey)
  {
    case Aspect_VKey_Escape: {
      QApplication::exit();
      return;
    }
    case Aspect_VKey_F: {
      // Temporarily hide guides from FitAll to avoid skewing zoom
      const bool hadX = !myAxisX.IsNull() && myContext->IsDisplayed(myAxisX);
      const bool hadY = !myAxisY.IsNull() && myContext->IsDisplayed(myAxisY);
      if (hadX) myContext->Erase(myAxisX, false);
      if (hadY) myContext->Erase(myAxisY, false);
      myView->FitAll(0.01, false);
      if (hadX) myContext->Display(myAxisX, 0, 1, false);
      if (hadY) myContext->Display(myAxisY, 0, 1, false);
      update();
      return;
    }
  }
  QOpenGLWidget::keyPressEvent(theEvent);
}

// ================================================================
// Function : mousePressEvent
// ================================================================
void OcctQOpenGLWidgetViewer::mousePressEvent(QMouseEvent* theEvent)
{
  QOpenGLWidget::mousePressEvent(theEvent);
  if (myView.IsNull())
    return;

  const qreal            aPixelRatio = devicePixelRatioF();
  const Graphic3d_Vec2i  aPnt(theEvent->pos().x() * aPixelRatio, theEvent->pos().y() * aPixelRatio);
  const Aspect_VKeyFlags aFlags = OcctQtTools::qtMouseModifiers2VKeys(theEvent->modifiers());
  if (UpdateMouseButtons(aPnt, OcctQtTools::qtMouseButtons2VKeys(theEvent->buttons()), aFlags, false))
    updateView();
}

// ================================================================
// Function : mouseReleaseEvent
// ================================================================
void OcctQOpenGLWidgetViewer::mouseReleaseEvent(QMouseEvent* theEvent)
{
  QOpenGLWidget::mouseReleaseEvent(theEvent);
  if (myView.IsNull())
    return;

  const qreal            aPixelRatio = devicePixelRatioF();
  const Graphic3d_Vec2i  aPnt(theEvent->pos().x() * aPixelRatio, theEvent->pos().y() * aPixelRatio);
  const Aspect_VKeyFlags aFlags = OcctQtTools::qtMouseModifiers2VKeys(theEvent->modifiers());
  if (UpdateMouseButtons(aPnt, OcctQtTools::qtMouseButtons2VKeys(theEvent->buttons()), aFlags, false))
    updateView();
}

// ================================================================
// Function : mouseMoveEvent
// ================================================================
void OcctQOpenGLWidgetViewer::mouseMoveEvent(QMouseEvent* theEvent)
{
  QOpenGLWidget::mouseMoveEvent(theEvent);
  if (myView.IsNull())
    return;

  const qreal           aPixelRatio = devicePixelRatioF();
  const Graphic3d_Vec2i aNewPos(theEvent->pos().x() * aPixelRatio, theEvent->pos().y() * aPixelRatio);
  if (UpdateMousePosition(aNewPos,
                          OcctQtTools::qtMouseButtons2VKeys(theEvent->buttons()),
                          OcctQtTools::qtMouseModifiers2VKeys(theEvent->modifiers()),
                          false))
  {
    updateView();
  }
}

// ==============================================================================
// function : wheelEvent
// ==============================================================================
void OcctQOpenGLWidgetViewer::wheelEvent(QWheelEvent* theEvent)
{
  QOpenGLWidget::wheelEvent(theEvent);
  if (myView.IsNull())
    return;

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
  const qreal           aPixelRatio = devicePixelRatioF();
  const Graphic3d_Vec2i aPos(
    Graphic3d_Vec2d(theEvent->position().x() * aPixelRatio, theEvent->position().y() * aPixelRatio));
#else
  const qreal           aPixelRatio = devicePixelRatioF();
  const Graphic3d_Vec2i aPos(theEvent->pos().x() * aPixelRatio, theEvent->pos().y() * aPixelRatio);
#endif
  if (!myView->Subviews().IsEmpty())
  {
    Handle(V3d_View) aPickedView = myView->PickSubview(aPos);
    if (!aPickedView.IsNull() && aPickedView != myFocusView)
    {
      // switch input focus to another subview
      OnSubviewChanged(myContext, myFocusView, aPickedView);
      updateView();
      return;
    }
  }

  if (UpdateZoom(Aspect_ScrollDelta(aPos, double(theEvent->angleDelta().y()) / 8.0)))
  {
    // Keep grid spacing consistent in screen space while zooming
    updateGridStepForView(!myFocusView.IsNull() ? myFocusView : myView);
    updateView();
  }
}

// =======================================================================
// function : updateView
// =======================================================================
void OcctQOpenGLWidgetViewer::updateView()
{
  update();
  // if (window() != NULL) { window()->update(); }
}

// ================================================================
// Function : paintGL
// ================================================================
void OcctQOpenGLWidgetViewer::paintGL()
{
  if (myView.IsNull() || myView->Window().IsNull())
    return;

  Aspect_Drawable aNativeWin = (Aspect_Drawable)winId();
#ifdef _WIN32
  HDC  aWglDevCtx = wglGetCurrentDC();
  HWND aWglWin    = WindowFromDC(aWglDevCtx);
  aNativeWin      = (Aspect_Drawable)aWglWin;
#endif
  if (myView->Window()->NativeHandle() != aNativeWin)
  {
    // workaround window recreation done by Qt on monitor (QScreen) disconnection
    Message::SendWarning() << "Native window handle has changed by QOpenGLWidget!";
    initializeGL();
    return;
  }

  // wrap FBO created by QOpenGLWidget
  // get context from this (composer) view rather than from arbitrary one
  // Handle(OpenGl_GraphicDriver) aDriver =
  //   Handle(OpenGl_GraphicDriver)::DownCast(myContext->CurrentViewer()->Driver());
  // Handle(OpenGl_Context) aGlCtx = aDriver->GetSharedContext();
  Handle(OpenGl_Context)     aGlCtx      = OcctGlTools::GetGlContext(myView);
  Handle(OpenGl_FrameBuffer) aDefaultFbo = aGlCtx->DefaultFrameBuffer();
  if (aDefaultFbo.IsNull())
  {
    aDefaultFbo = new OcctQtFrameBuffer();
    aGlCtx->SetDefaultFrameBuffer(aDefaultFbo);
  }
  if (!aDefaultFbo->InitWrapper(aGlCtx))
  {
    aDefaultFbo.Nullify();
    Message::DefaultMessenger()->Send("Default FBO wrapper creation failed", Message_Fail);
    QMessageBox::critical(0, "Failure", "Default FBO wrapper creation failed");
    QApplication::exit(1);
    return;
  }

  Graphic3d_Vec2i aViewSizeOld;
  // don't use QWidget::rect() as it might return misleading information
  // const QRect aRect = rect();
  // Graphic3d_Vec2i aViewSizeNew(aRect.right() - aRect.left(), aRect.bottom() - aRect.top());
  const Graphic3d_Vec2i        aViewSizeNew = aDefaultFbo->GetVPSize();
  Handle(Aspect_NeutralWindow) aWindow      = Handle(Aspect_NeutralWindow)::DownCast(myView->Window());
  aWindow->Size(aViewSizeOld.x(), aViewSizeOld.y());
  if (aViewSizeNew != aViewSizeOld)
  {
    aWindow->SetSize(aViewSizeNew.x(), aViewSizeNew.y());
    myView->MustBeResized();
    myView->Invalidate();
    dumpGlInfo(true, false);

    for (const Handle(V3d_View)& aSubviewIter : myView->Subviews())
    {
      aSubviewIter->MustBeResized();
      aSubviewIter->Invalidate();
      aDefaultFbo->SetupViewport(aGlCtx);
    }
  }

  // flush pending input events and redraw the viewer
  Handle(V3d_View) aView = !myFocusView.IsNull() ? myFocusView : myView;
  aView->InvalidateImmediate();
  FlushViewEvents(myContext, aView, true);
}

// ================================================================
// Function : handleViewRedraw
// ================================================================
void OcctQOpenGLWidgetViewer::handleViewRedraw(const Handle(AIS_InteractiveContext)& theCtx,
                                               const Handle(V3d_View)&               theView)
{
  AIS_ViewController::handleViewRedraw(theCtx, theView);
  // Adjust grid dynamically in response to any view change (pan/rotate/zoom)
  updateGridStepForView(theView);
  if (myToAskNextFrame)
    updateView(); // ask more frames for animation
}

// ================================================================
// Function : addBody
// ================================================================
Handle(AIS_Shape) OcctQOpenGLWidgetViewer::addBody(const TopoDS_Shape& theShape,
                                                   AIS_DisplayMode    theDispMode,
                                                   Standard_Integer   theDispPriority,
                                                   bool               theToUpdate)
{
  Handle(AIS_Shape) aShape = new AIS_Shape(theShape);
  myBodies.Append(aShape);
  myContext->Display(aShape, theDispMode, theDispPriority, theToUpdate);
  return aShape;
}

// ================================================================
// Function : setBodiesVisible
// ================================================================
void OcctQOpenGLWidgetViewer::setBodiesVisible(bool theVisible)
{
  if (myBodiesVisible == theVisible)
    return;
  myBodiesVisible = theVisible;
  for (NCollection_Sequence<Handle(AIS_Shape)>::Iterator it(myBodies); it.More(); it.Next())
  {
    const Handle(AIS_Shape)& aBody = it.Value();
    if (aBody.IsNull())
      continue;
    if (theVisible)
    {
      if (!myContext->IsDisplayed(aBody))
        myContext->Display(aBody, 0, 0, false);
      // Ensure shaded mode when showing back
      myContext->SetDisplayMode(aBody, AIS_Shaded, false);
    }
    else
    {
      if (myContext->IsDisplayed(aBody))
        myContext->Erase(aBody, false);
    }
  }
  // keep guides visible regardless of toggle
  if (!myAxisX.IsNull()) myContext->Display(myAxisX, 0, 1, false);
  if (!myAxisY.IsNull()) myContext->Display(myAxisY, 0, 1, false);
  if (!myOriginTrihedron.IsNull()) myContext->Display(myOriginTrihedron, 0, 2, false);
  if (!myViewCube.IsNull()) myContext->Display(myViewCube, 0, 0, false);
  // Force viewer update so changes are visible immediately
  if (!myView.IsNull())
  {
    myContext->UpdateCurrentViewer();
    myView->Invalidate();
  }
  update();
}

// ================================================================
// Function : toggleBodiesVisible
// ================================================================
bool OcctQOpenGLWidgetViewer::toggleBodiesVisible()
{
  setBodiesVisible(!myBodiesVisible);
  return myBodiesVisible;
}

// ================================================================
// Function : rayHitZ0
// ================================================================
bool OcctQOpenGLWidgetViewer::rayHitZ0(const Handle(V3d_View)& theView,
                                       int                     thePx,
                                       int                     thePy,
                                       gp_Pnt&                 theHit) const
{
  if (theView.IsNull())
    return false;

  Standard_Real X = 0.0, Y = 0.0, Z = 0.0;
  Standard_Real Vx = 0.0, Vy = 0.0, Vz = 0.0;
  theView->ConvertWithProj(thePx, thePy, X, Y, Z, Vx, Vy, Vz);
  if (Abs(Vz) < 1.0e-12)
    return false; // parallel to Z=0

  const Standard_Real t = -Z / Vz;
  theHit.SetCoord(X + t * Vx, Y + t * Vy, 0.0);
  return Standard_True;
}

// ================================================================
// Function : updateGridStepForView
// ================================================================
void OcctQOpenGLWidgetViewer::updateGridStepForView(const Handle(V3d_View)& theView)
{
  if (theView.IsNull() || myViewer.IsNull())
    return;

  // Determine viewport size in pixels
  Handle(Aspect_NeutralWindow) aWnd = Handle(Aspect_NeutralWindow)::DownCast(theView->Window());
  if (aWnd.IsNull())
    return;

  Standard_Integer vpW = 0, vpH = 0;
  aWnd->Size(vpW, vpH);
  if (vpW <= 0 || vpH <= 0)
    return;

  // Compute world distance corresponding to ~20 pixels at view center on Z=0 plane
  const int cx = vpW / 2;
  const int cy = vpH / 2;
  gp_Pnt p0, p1;
  if (!rayHitZ0(theView, cx, cy, p0) || !rayHitZ0(theView, cx + 20, cy, p1))
    return;

  const double pixelsTarget = 22.0; // desired spacing in pixels
  const double worldPer20px = p0.Distance(p1);
  if (worldPer20px <= 0.0)
    return;

  const double worldPerPixel = worldPer20px / 20.0;
  double       desiredStep   = worldPerPixel * pixelsTarget;

  // Snap to nice 1-2-5 sequence (… 0.1, 0.2, 0.5, 1, 2, 5, …)
  const double minStep = 1.0e-4; // 0.0001 in model units (~0.1 mm if unit=mm)
  if (desiredStep < minStep)
    desiredStep = minStep;
  const double maxStep = 1.0e+3; // clamp to keep grid visible when zoomed out
  if (desiredStep > maxStep)
    desiredStep = maxStep;

  const double pow10   = pow(10.0, floor(log10(desiredStep)));
  const double norm    = desiredStep / pow10;
  double       snapped = 1.0 * pow10;
  if (norm <= 1.0)
    snapped = 1.0 * pow10;
  else if (norm <= 2.0)
    snapped = 2.0 * pow10;
  else if (norm <= 5.0)
    snapped = 5.0 * pow10;
  else
    snapped = 10.0 * pow10;

  // Avoid jittering updates for tiny differences
  if (Abs(snapped - myGridStep) < (1.0e-6 * Max(1.0, myGridStep)))
    return;

  myGridStep = snapped;
  myViewer->SetRectangularGridValues(0.0, 0.0, myGridStep, myGridStep, 0.0);
  // Ensure grid is active
  myViewer->ActivateGrid(Aspect_GT_Rectangular, Aspect_GDM_Lines);
}

// ================================================================
// Function : OnSubviewChanged
// ================================================================
void OcctQOpenGLWidgetViewer::OnSubviewChanged(const Handle(AIS_InteractiveContext)&,
                                               const Handle(V3d_View)&,
                                               const Handle(V3d_View)& theNewView)
{
  myFocusView = theNewView;
}
