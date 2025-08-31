#include "OcctQOpenGLWidgetViewer.h"
#include "OcctGlTools.h"
#include "OcctQtTools.h"

#include <Standard_WarningsDisable.hxx>
#include <QApplication>
#include <QMessageBox>
#include <QMouseEvent>
#include <QSurfaceFormat>
#include <Standard_WarningsRestore.hxx>

#include <OpenGl_Context.hxx>
#include <OpenGl_FrameBuffer.hxx>
#include <OpenGl_GraphicDriver.hxx>

#include <AIS_Shape.hxx>
#include <AIS_ViewCube.hxx>
#include <AIS_Line.hxx>
#include <AIS_Trihedron.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <Aspect_NeutralWindow.hxx>
#include <Geom_Axis2Placement.hxx>
#include <Geom_Line.hxx>
#include <gp_Ax2.hxx>
#include <Prs3d_Drawer.hxx>
#include <Prs3d_DatumAspect.hxx>
#include <Prs3d_LineAspect.hxx>
#include <Prs3d_TypeOfHighlight.hxx>
#include <Quantity_Color.hxx>
#include <Standard_Version.hxx>
#include <V3d_RectangularGrid.hxx>
#include <gp_Pnt.hxx>

class OcctQtFrameBuffer : public OpenGl_FrameBuffer
{
  DEFINE_STANDARD_RTTI_INLINE(OcctQtFrameBuffer, OpenGl_FrameBuffer)
public:
  OcctQtFrameBuffer() {}
  virtual void BindBuffer(const Handle(OpenGl_Context)& theGlCtx) override
  {
    OpenGl_FrameBuffer::BindBuffer(theGlCtx);
    theGlCtx->SetFrameBufferSRGB(true, false);
  }
  virtual void BindDrawBuffer(const Handle(OpenGl_Context)& theGlCtx) override
  {
    OpenGl_FrameBuffer::BindDrawBuffer(theGlCtx);
    theGlCtx->SetFrameBufferSRGB(true, false);
  }
  virtual void BindReadBuffer(const Handle(OpenGl_Context)& theGlCtx) override
  {
    OpenGl_FrameBuffer::BindReadBuffer(theGlCtx);
  }
};

OcctQOpenGLWidgetViewer::OcctQOpenGLWidgetViewer(QWidget* theParent)
  : QOpenGLWidget(theParent)
{
  Handle(Aspect_DisplayConnection) aDisp   = new Aspect_DisplayConnection();
  Handle(OpenGl_GraphicDriver)     aDriver = new OpenGl_GraphicDriver(aDisp, false);
  aDriver->ChangeOptions().buffersNoSwap = true;
  aDriver->ChangeOptions().buffersOpaqueAlpha = true;
  aDriver->ChangeOptions().useSystemBuffer = false;

  myViewer = new V3d_Viewer(aDriver);
  const Quantity_Color aBgTop(0.24, 0.24, 0.24, Quantity_TOC_sRGB);
  const Quantity_Color aBgBottom(0.36, 0.36, 0.36, Quantity_TOC_sRGB);
  myViewer->SetDefaultBackgroundColor(aBgTop);
  myViewer->SetDefaultBgGradientColors(aBgTop, aBgBottom, Aspect_GradientFillMethod_Elliptical);
  myViewer->SetDefaultLights();
  myViewer->SetLightOn();
  myGridStep = 10.0;
  myViewer->SetRectangularGridValues(0.0, 0.0, myGridStep, myGridStep, 0.0);
  myViewer->ActivateGrid(Aspect_GT_Rectangular, Aspect_GDM_Lines);

  myContext = new AIS_InteractiveContext(myViewer);

  // Tweak highlight styles for better feedback
  {
    Handle(Prs3d_Drawer) aSel = myContext->SelectionStyle();
    if (!aSel.IsNull())
    {
      aSel->SetColor(Quantity_NOC_ORANGE);
      aSel->SetDisplayMode(AIS_Shaded);
      aSel->SetTransparency(0.2f);
    }
    Handle(Prs3d_Drawer) aDyn = myContext->HighlightStyle(Prs3d_TypeOfHighlight_Dynamic);
    if (!aDyn.IsNull())
    {
      aDyn->SetColor(Quantity_NOC_CYAN1);
      aDyn->SetDisplayMode(AIS_Shaded);
      aDyn->SetTransparency(0.7f);
    }
  }

  myViewCube = new AIS_ViewCube();
  myViewCube->SetViewAnimation(myViewAnimation);
  myViewCube->SetFixedAnimationLoop(false);
  myViewCube->SetAutoStartAnimation(true);
  myViewCube->SetSize(60.0);
  myViewCube->SetBoxColor(Quantity_NOC_GRAY70);
  myViewCube->TransformPersistence()->SetCorner2d(Aspect_TOTP_RIGHT_UPPER);
  myViewCube->TransformPersistence()->SetOffset2d(Graphic3d_Vec2i(80, 80));

  myView = myViewer->CreateView();
  myView->SetBgGradientColors(aBgTop, aBgBottom, Aspect_GradientFillMethod_Elliptical);
  myView->SetImmediateUpdate(false);
#ifndef __APPLE__
  myView->ChangeRenderingParams().NbMsaaSamples = 4;
#endif
  myView->ChangeRenderingParams().ToShowStats = true;
  myView->ChangeRenderingParams().StatsTextHeight = 24;
  myView->ChangeRenderingParams().CollectedStats = (Graphic3d_RenderingParams::PerfCounters)(
    Graphic3d_RenderingParams::PerfCounters_FrameRate | Graphic3d_RenderingParams::PerfCounters_Triangles);

  setMouseTracking(true);
  setBackgroundRole(QPalette::NoRole);
  setFocusPolicy(Qt::StrongFocus);
  setUpdatesEnabled(true);
  setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);

  QSurfaceFormat aGlFormat;
  aGlFormat.setDepthBufferSize(24);
  aGlFormat.setStencilBufferSize(8);
  aDriver->ChangeOptions().contextDebug = aGlFormat.testOption(QSurfaceFormat::DebugContext);
  if (myIsCoreProfile) aGlFormat.setVersion(4, 5);
  aGlFormat.setProfile(myIsCoreProfile ? QSurfaceFormat::CoreProfile : QSurfaceFormat::CompatibilityProfile);
  setFormat(aGlFormat);
#if defined(_WIN32)
  QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
#endif
}

OcctQOpenGLWidgetViewer::~OcctQOpenGLWidgetViewer()
{
  Handle(Aspect_DisplayConnection) aDisp = myViewer->Driver()->GetDisplayConnection();
  myContext->RemoveAll(false);
  myContext.Nullify();
  myView->Remove();
  myView.Nullify();
  myViewer.Nullify();
  makeCurrent();
  aDisp.Nullify();
}

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
      if (!anInfo.IsEmpty()) anInfo += "\n";
      anInfo += aValueIter.Key() + ": " + aValueIter.Value();
    }
  }
  if (theToPrint) Message::SendInfo(anInfo);
  myGlInfo = QString::fromUtf8(anInfo.ToCString());
}

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
    // guides and origin
    const Standard_Real L = 1000.0;
    Handle(Geom_Line) aGeomX = new Geom_Line(gp_Pnt(-L, 0.0, 0.0), gp_Dir(1.0, 0.0, 0.0));
    Handle(Geom_Line) aGeomY = new Geom_Line(gp_Pnt(0.0, -L, 0.0), gp_Dir(0.0, 1.0, 0.0));
    myAxisX = new AIS_Line(aGeomX);
    myAxisY = new AIS_Line(aGeomY);
    myAxisX->SetColor(Quantity_NOC_RED);
    myAxisY->SetColor(Quantity_NOC_GREEN);
    Handle(Prs3d_Drawer) xDr = myAxisX->Attributes(); if (xDr.IsNull()) xDr = new Prs3d_Drawer();
    xDr->SetLineAspect(new Prs3d_LineAspect(Quantity_NOC_RED, Aspect_TOL_SOLID, 2.0f));
    myAxisX->SetAttributes(xDr);
    Handle(Prs3d_Drawer) yDr = myAxisY->Attributes(); if (yDr.IsNull()) yDr = new Prs3d_Drawer();
    yDr->SetLineAspect(new Prs3d_LineAspect(Quantity_NOC_GREEN, Aspect_TOL_SOLID, 2.0f));
    myAxisY->SetAttributes(yDr);
    myContext->Display(myAxisX, 0, 1, false);
    myContext->Display(myAxisY, 0, 1, false);

    myOriginPlacement = new Geom_Axis2Placement(gp_Ax2(gp::Origin(), gp::DZ(), gp::DX()));
    myOriginTrihedron = new AIS_Trihedron(myOriginPlacement);
    Handle(Prs3d_Drawer) trDr = myOriginTrihedron->Attributes(); if (trDr.IsNull()) trDr = new Prs3d_Drawer();
    trDr->SetDatumAspect(new Prs3d_DatumAspect());
    trDr->DatumAspect()->SetAxisLength(20.0, 20.0, 20.0);
    myOriginTrihedron->SetAttributes(trDr);
    myContext->Display(myOriginTrihedron, 0, 2, false);
    updateGridStepForView(myView);
  }
}

void OcctQOpenGLWidgetViewer::closeEvent(QCloseEvent* theEvent)
{
  theEvent->accept();
}

void OcctQOpenGLWidgetViewer::keyPressEvent(QKeyEvent* theEvent)
{
  if (myView.IsNull()) return;
  const Aspect_VKey aKey = OcctQtTools::qtKey2VKey(theEvent->key());
  switch (aKey)
  {
    case Aspect_VKey_Escape: QApplication::exit(); return;
    case Aspect_VKey_F: {
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
    default: break;
  }
  QOpenGLWidget::keyPressEvent(theEvent);
}

void OcctQOpenGLWidgetViewer::mousePressEvent(QMouseEvent* theEvent)
{
  QOpenGLWidget::mousePressEvent(theEvent);
  if (myView.IsNull()) return;
  const qreal aPixelRatio = devicePixelRatioF();
  const Graphic3d_Vec2i aPnt(theEvent->pos().x() * aPixelRatio, theEvent->pos().y() * aPixelRatio);
  const Aspect_VKeyFlags aFlags = OcctQtTools::qtMouseModifiers2VKeys(theEvent->modifiers());
  if (UpdateMouseButtons(aPnt, OcctQtTools::qtMouseButtons2VKeys(theEvent->buttons()), aFlags, false))
    updateView();
}

void OcctQOpenGLWidgetViewer::mouseReleaseEvent(QMouseEvent* theEvent)
{
  QOpenGLWidget::mouseReleaseEvent(theEvent);
  if (myView.IsNull()) return;
  const qreal aPixelRatio = devicePixelRatioF();
  const Graphic3d_Vec2i aPnt(theEvent->pos().x() * aPixelRatio, theEvent->pos().y() * aPixelRatio);
  const Aspect_VKeyFlags aFlags = OcctQtTools::qtMouseModifiers2VKeys(theEvent->modifiers());
  if (UpdateMouseButtons(aPnt, OcctQtTools::qtMouseButtons2VKeys(theEvent->buttons()), aFlags, false))
    updateView();
  // Selection: ensure exactly one is selected on click, no toggling/accumulation
  if (!myContext.IsNull() && myContext->HasDetected())
  {
    Handle(AIS_Shape) det = Handle(AIS_Shape)::DownCast(myContext->DetectedInteractive());
    myContext->ClearSelected(false);
    if (!det.IsNull())
    {
      myContext->AddOrRemoveSelected(det, false);
    }
  }
  else
  {
    myContext->ClearSelected(false);
  }
  if (!myView.IsNull()) { myContext->UpdateCurrentViewer(); myView->Invalidate(); }
  update();
}

void OcctQOpenGLWidgetViewer::mouseMoveEvent(QMouseEvent* theEvent)
{
  QOpenGLWidget::mouseMoveEvent(theEvent);
  if (myView.IsNull()) return;
  const qreal aPixelRatio = devicePixelRatioF();
  const Graphic3d_Vec2i aNewPos(theEvent->pos().x() * aPixelRatio, theEvent->pos().y() * aPixelRatio);
  if (UpdateMousePosition(aNewPos,
                          OcctQtTools::qtMouseButtons2VKeys(theEvent->buttons()),
                          OcctQtTools::qtMouseModifiers2VKeys(theEvent->modifiers()),
                          false))
  {
    updateView();
  }
  // no-op: we don't cache last detected anymore
}

void OcctQOpenGLWidgetViewer::wheelEvent(QWheelEvent* theEvent)
{
  QOpenGLWidget::wheelEvent(theEvent);
  if (myView.IsNull()) return;
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
  const qreal aPixelRatio = devicePixelRatioF();
  const Graphic3d_Vec2i aPos(Graphic3d_Vec2d(theEvent->position().x() * aPixelRatio,
                                             theEvent->position().y() * aPixelRatio));
#else
  const qreal aPixelRatio = devicePixelRatioF();
  const Graphic3d_Vec2i aPos(theEvent->pos().x() * aPixelRatio, theEvent->pos().y() * aPixelRatio);
#endif
  if (!myView->Subviews().IsEmpty())
  {
    Handle(V3d_View) aPickedView = myView->PickSubview(aPos);
    if (!aPickedView.IsNull() && aPickedView != myFocusView)
    {
      OnSubviewChanged(myContext, myFocusView, aPickedView);
      updateView();
      return;
    }
  }
  if (UpdateZoom(Aspect_ScrollDelta(aPos, double(theEvent->angleDelta().y()) / 8.0)))
  {
    updateGridStepForView(!myFocusView.IsNull() ? myFocusView : myView);
    updateView();
  }
}

void OcctQOpenGLWidgetViewer::updateView()
{
  update();
}

void OcctQOpenGLWidgetViewer::paintGL()
{
  if (myView.IsNull() || myView->Window().IsNull()) return;
  Aspect_Drawable aNativeWin = (Aspect_Drawable)winId();
#ifdef _WIN32
  HDC  aWglDevCtx = wglGetCurrentDC();
  HWND aWglWin    = WindowFromDC(aWglDevCtx);
  aNativeWin      = (Aspect_Drawable)aWglWin;
#endif
  if (myView->Window()->NativeHandle() != aNativeWin)
  {
    initializeGL();
    return;
  }
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
  const Graphic3d_Vec2i aViewSizeNew = aDefaultFbo->GetVPSize();
  Handle(Aspect_NeutralWindow) aWindow = Handle(Aspect_NeutralWindow)::DownCast(myView->Window());
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
  Handle(V3d_View) aView = !myFocusView.IsNull() ? myFocusView : myView;
  aView->InvalidateImmediate();
  FlushViewEvents(myContext, aView, true);
}

void OcctQOpenGLWidgetViewer::handleViewRedraw(const Handle(AIS_InteractiveContext)& theCtx,
                                               const Handle(V3d_View)& theView)
{
  AIS_ViewController::handleViewRedraw(theCtx, theView);
  updateGridStepForView(theView);
  if (myToAskNextFrame) updateView();
}

bool OcctQOpenGLWidgetViewer::rayHitZ0(const Handle(V3d_View)& theView, int thePx, int thePy, gp_Pnt& theHit) const
{
  if (theView.IsNull()) return false;
  Standard_Real X = 0.0, Y = 0.0, Z = 0.0;
  Standard_Real Vx = 0.0, Vy = 0.0, Vz = 0.0;
  theView->ConvertWithProj(thePx, thePy, X, Y, Z, Vx, Vy, Vz);
  if (Abs(Vz) < 1.0e-12) return false;
  const Standard_Real t = -Z / Vz;
  theHit.SetCoord(X + t * Vx, Y + t * Vy, 0.0);
  return Standard_True;
}

void OcctQOpenGLWidgetViewer::updateGridStepForView(const Handle(V3d_View)& theView)
{
  if (theView.IsNull() || myViewer.IsNull()) return;
  Handle(Aspect_NeutralWindow) aWnd = Handle(Aspect_NeutralWindow)::DownCast(theView->Window());
  if (aWnd.IsNull()) return;
  Standard_Integer vpW = 0, vpH = 0; aWnd->Size(vpW, vpH);
  if (vpW <= 0 || vpH <= 0) return;
  const int cx = vpW / 2; const int cy = vpH / 2; gp_Pnt p0, p1;
  if (!rayHitZ0(theView, cx, cy, p0) || !rayHitZ0(theView, cx + 20, cy, p1)) return;
  const double pixelsTarget = 22.0;
  const double worldPer20px = p0.Distance(p1); if (worldPer20px <= 0.0) return;
  const double worldPerPixel = worldPer20px / 20.0; double desiredStep = worldPerPixel * pixelsTarget;
  const double minStep = 1.0e-4; if (desiredStep < minStep) desiredStep = minStep;
  const double maxStep = 1.0e+3; if (desiredStep > maxStep) desiredStep = maxStep;
  const double pow10 = pow(10.0, floor(log10(desiredStep))); const double norm = desiredStep / pow10;
  double snapped = 1.0 * pow10;
  if (norm <= 1.0) snapped = 1.0 * pow10; else if (norm <= 2.0) snapped = 2.0 * pow10;
  else if (norm <= 5.0) snapped = 5.0 * pow10; else snapped = 10.0 * pow10;
  if (Abs(snapped - myGridStep) < (1.0e-6 * Max(1.0, myGridStep))) return;
  myGridStep = snapped;
  myViewer->SetRectangularGridValues(0.0, 0.0, myGridStep, myGridStep, 0.0);
  myViewer->ActivateGrid(Aspect_GT_Rectangular, Aspect_GDM_Lines);
}

void OcctQOpenGLWidgetViewer::OnSubviewChanged(const Handle(AIS_InteractiveContext)&,
                                               const Handle(V3d_View)&,
                                               const Handle(V3d_View)& theNewView)
{
  myFocusView = theNewView;
}

Handle(AIS_Shape) OcctQOpenGLWidgetViewer::addBody(const TopoDS_Shape& theShape,
                                                   AIS_DisplayMode theDispMode,
                                                   Standard_Integer theDispPriority,
                                                   bool theToUpdate)
{
  Handle(AIS_Shape) aShape = new AIS_Shape(theShape);
  myBodies.Append(aShape);
  myContext->Display(aShape, theDispMode, theDispPriority, theToUpdate);
  return aShape;
}

void OcctQOpenGLWidgetViewer::clearBodies(bool theToUpdate)
{
  for (NCollection_Sequence<Handle(AIS_Shape)>::Iterator it(myBodies); it.More(); it.Next())
  {
    const Handle(AIS_Shape)& aBody = it.Value();
    if (!aBody.IsNull() && myContext->IsDisplayed(aBody))
    {
      myContext->Erase(aBody, false);
    }
  }
  myBodies.Clear();
  if (theToUpdate)
  {
    if (!myView.IsNull()) { myContext->UpdateCurrentViewer(); myView->Invalidate(); }
    update();
  }
}

// setBodiesVisible / toggleBodiesVisible removed per UI simplification
Handle(AIS_Shape) OcctQOpenGLWidgetViewer::selectedShape() const
{
  Handle(AIS_Shape) result;
  for (myContext->InitSelected(); myContext->MoreSelected(); myContext->NextSelected())
  {
    result = Handle(AIS_Shape)::DownCast(myContext->SelectedInteractive());
    if (!result.IsNull()) break;
  }
  return result;
}

Handle(AIS_Shape) OcctQOpenGLWidgetViewer::detectedShape() const
{
  if (!myContext.IsNull() && myContext->HasDetected())
  {
    return Handle(AIS_Shape)::DownCast(myContext->DetectedInteractive());
  }
  return Handle(AIS_Shape)();
}
