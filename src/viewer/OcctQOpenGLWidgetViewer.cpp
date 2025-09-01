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

  m_viewer = new V3d_Viewer(aDriver);
  const Quantity_Color aBgTop(0.24, 0.24, 0.24, Quantity_TOC_sRGB);
  const Quantity_Color aBgBottom(0.36, 0.36, 0.36, Quantity_TOC_sRGB);
  m_viewer->SetDefaultBackgroundColor(aBgTop);
  m_viewer->SetDefaultBgGradientColors(aBgTop, aBgBottom, Aspect_GradientFillMethod_Elliptical);
  m_viewer->SetDefaultLights();
  m_viewer->SetLightOn();
  m_gridStep = 10.0;
  m_viewer->SetRectangularGridValues(0.0, 0.0, m_gridStep, m_gridStep, 0.0);
  m_viewer->ActivateGrid(Aspect_GT_Rectangular, Aspect_GDM_Lines);

  m_context = new AIS_InteractiveContext(m_viewer);

  // Tweak highlight styles for better feedback
  {
    Handle(Prs3d_Drawer) aSel = m_context->SelectionStyle();
    if (!aSel.IsNull())
    {
      aSel->SetColor(Quantity_NOC_ORANGE);
      aSel->SetDisplayMode(AIS_Shaded);
      aSel->SetTransparency(0.2f);
    }
    Handle(Prs3d_Drawer) aDyn = m_context->HighlightStyle(Prs3d_TypeOfHighlight_Dynamic);
    if (!aDyn.IsNull())
    {
      aDyn->SetColor(Quantity_NOC_CYAN1);
      aDyn->SetDisplayMode(AIS_Shaded);
      aDyn->SetTransparency(0.7f);
    }
  }

  m_viewCube = new AIS_ViewCube();
  m_viewCube->SetViewAnimation(myViewAnimation);
  m_viewCube->SetFixedAnimationLoop(false);
  m_viewCube->SetAutoStartAnimation(true);
  m_viewCube->SetSize(60.0);
  m_viewCube->SetBoxColor(Quantity_NOC_GRAY70);
  m_viewCube->TransformPersistence()->SetCorner2d(Aspect_TOTP_RIGHT_UPPER);
  m_viewCube->TransformPersistence()->SetOffset2d(Graphic3d_Vec2i(80, 80));

  m_view = m_viewer->CreateView();
  m_view->SetBgGradientColors(aBgTop, aBgBottom, Aspect_GradientFillMethod_Elliptical);
  m_view->SetImmediateUpdate(false);
#ifndef __APPLE__
  m_view->ChangeRenderingParams().NbMsaaSamples = 4;
#endif
  m_view->ChangeRenderingParams().ToShowStats = true;
  m_view->ChangeRenderingParams().StatsTextHeight = 24;
  m_view->ChangeRenderingParams().CollectedStats = (Graphic3d_RenderingParams::PerfCounters)(
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
  if (m_isCoreProfile) aGlFormat.setVersion(4, 5);
  aGlFormat.setProfile(m_isCoreProfile ? QSurfaceFormat::CoreProfile : QSurfaceFormat::CompatibilityProfile);
  setFormat(aGlFormat);
#if defined(_WIN32)
  QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
#endif
}

OcctQOpenGLWidgetViewer::~OcctQOpenGLWidgetViewer()
{
  // Properly release OCCT resources bound to Qt GL context
  Handle(Aspect_DisplayConnection) aDisp = m_viewer->Driver()->GetDisplayConnection();
  m_context->RemoveAll(false);
  m_context.Nullify();
  m_view->Remove();
  m_view.Nullify();
  m_viewer.Nullify();
  makeCurrent();
  aDisp.Nullify();
}

void OcctQOpenGLWidgetViewer::dumpGlInfo(bool theIsBasic, bool theToPrint)
{
  // Collect key GL capabilities into a single text blob
  TColStd_IndexedDataMapOfStringString aGlCapsDict;
  m_view->DiagnosticInformation(aGlCapsDict,
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
  m_glInfo = QString::fromUtf8(anInfo.ToCString());
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

  // Wrap current Qt OpenGL context for OCCT rendering
  Handle(OpenGl_Context) aGlCtx = new OpenGl_Context();
  if (!aGlCtx->Init(m_isCoreProfile))
  {
    Message::SendFail() << "Error: OpenGl_Context is unable to wrap OpenGL context";
    QMessageBox::critical(0, "Failure", "OpenGl_Context is unable to wrap OpenGL context");
    QApplication::exit(1);
    return;
  }

  // Bind OCCT neutral window to QWidget surface and size
  Handle(Aspect_NeutralWindow) aWindow = Handle(Aspect_NeutralWindow)::DownCast(m_view->Window());
  if (!aWindow.IsNull())
  {
    aWindow->SetNativeHandle(aNativeWin);
    aWindow->SetSize(aViewSize.x(), aViewSize.y());
    m_view->SetWindow(aWindow, aGlCtx->RenderingContext());
    dumpGlInfo(true, true);
  }
  else
  {
    aWindow = new Aspect_NeutralWindow();
    aWindow->SetVirtual(true);
    aWindow->SetNativeHandle(aNativeWin);
    aWindow->SetSize(aViewSize.x(), aViewSize.y());
    m_view->SetWindow(aWindow, aGlCtx->RenderingContext());
    dumpGlInfo(true, true);
    m_context->Display(m_viewCube, 0, 0, false);
  }

  {
    // Guides and origin gizmos (X/Y lines, origin trihedron)
    const Standard_Real L = 1000.0;
    Handle(Geom_Line) aGeomX = new Geom_Line(gp_Pnt(-L, 0.0, 0.0), gp_Dir(1.0, 0.0, 0.0));
    Handle(Geom_Line) aGeomY = new Geom_Line(gp_Pnt(0.0, -L, 0.0), gp_Dir(0.0, 1.0, 0.0));
    m_axisX = new AIS_Line(aGeomX);
    m_axisY = new AIS_Line(aGeomY);
    m_axisX->SetColor(Quantity_NOC_RED);
    m_axisY->SetColor(Quantity_NOC_GREEN);
    Handle(Prs3d_Drawer) xDr = m_axisX->Attributes(); if (xDr.IsNull()) xDr = new Prs3d_Drawer();
    xDr->SetLineAspect(new Prs3d_LineAspect(Quantity_NOC_RED, Aspect_TOL_SOLID, 2.0f));
    m_axisX->SetAttributes(xDr);
    Handle(Prs3d_Drawer) yDr = m_axisY->Attributes(); if (yDr.IsNull()) yDr = new Prs3d_Drawer();
    yDr->SetLineAspect(new Prs3d_LineAspect(Quantity_NOC_GREEN, Aspect_TOL_SOLID, 2.0f));
    m_axisY->SetAttributes(yDr);
    m_context->Display(m_axisX, 0, 1, false);
    m_context->Display(m_axisY, 0, 1, false);

    m_originPlacement = new Geom_Axis2Placement(gp_Ax2(gp::Origin(), gp::DZ(), gp::DX()));
    m_originTrihedron = new AIS_Trihedron(m_originPlacement);
    Handle(Prs3d_Drawer) trDr = m_originTrihedron->Attributes(); if (trDr.IsNull()) trDr = new Prs3d_Drawer();
    trDr->SetDatumAspect(new Prs3d_DatumAspect());
    trDr->DatumAspect()->SetAxisLength(20.0, 20.0, 20.0);
    m_originTrihedron->SetAttributes(trDr);
    m_context->Display(m_originTrihedron, 0, 2, false);
    updateGridStepForView(m_view);
  }
}

void OcctQOpenGLWidgetViewer::closeEvent(QCloseEvent* theEvent)
{
  theEvent->accept();
}

void OcctQOpenGLWidgetViewer::keyPressEvent(QKeyEvent* theEvent)
{
  if (m_view.IsNull()) return;
  const Aspect_VKey aKey = OcctQtTools::qtKey2VKey(theEvent->key());
  switch (aKey)
  {
    case Aspect_VKey_Escape: QApplication::exit(); return;
    case Aspect_VKey_F: {
      // Fit-all while preserving axis visibility
      const bool hadX = !m_axisX.IsNull() && m_context->IsDisplayed(m_axisX);
      const bool hadY = !m_axisY.IsNull() && m_context->IsDisplayed(m_axisY);
      if (hadX) m_context->Erase(m_axisX, false);
      if (hadY) m_context->Erase(m_axisY, false);
      m_view->FitAll(0.01, false);
      if (hadX) m_context->Display(m_axisX, 0, 1, false);
      if (hadY) m_context->Display(m_axisY, 0, 1, false);
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
  if (m_view.IsNull()) return;
  const qreal aPixelRatio = devicePixelRatioF();
  const Graphic3d_Vec2i aPnt(theEvent->pos().x() * aPixelRatio, theEvent->pos().y() * aPixelRatio);
  const Aspect_VKeyFlags aFlags = OcctQtTools::qtMouseModifiers2VKeys(theEvent->modifiers());
  if (UpdateMouseButtons(aPnt, OcctQtTools::qtMouseButtons2VKeys(theEvent->buttons()), aFlags, false))
    updateView();
}

void OcctQOpenGLWidgetViewer::mouseReleaseEvent(QMouseEvent* theEvent)
{
  QOpenGLWidget::mouseReleaseEvent(theEvent);
  if (m_view.IsNull()) return;
  const qreal aPixelRatio = devicePixelRatioF();
  const Graphic3d_Vec2i aPnt(theEvent->pos().x() * aPixelRatio, theEvent->pos().y() * aPixelRatio);
  const Aspect_VKeyFlags aFlags = OcctQtTools::qtMouseModifiers2VKeys(theEvent->modifiers());
  if (UpdateMouseButtons(aPnt, OcctQtTools::qtMouseButtons2VKeys(theEvent->buttons()), aFlags, false))
    updateView();
  // Selection: ensure exactly one is selected on click, no toggling/accumulation
  if (!m_context.IsNull() && m_context->HasDetected())
  {
    Handle(AIS_Shape) det = Handle(AIS_Shape)::DownCast(m_context->DetectedInteractive());
    m_context->ClearSelected(false);
    if (!det.IsNull())
    {
      m_context->AddOrRemoveSelected(det, false);
    }
  }
  else
  {
    m_context->ClearSelected(false);
  }
  if (!m_view.IsNull()) { m_context->UpdateCurrentViewer(); m_view->Invalidate(); }
  update();
  emit selectionChanged();
}

void OcctQOpenGLWidgetViewer::mouseMoveEvent(QMouseEvent* theEvent)
{
  QOpenGLWidget::mouseMoveEvent(theEvent);
  if (m_view.IsNull()) return;
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
  if (m_view.IsNull()) return;
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
  const qreal aPixelRatio = devicePixelRatioF();
  const Graphic3d_Vec2i aPos(Graphic3d_Vec2d(theEvent->position().x() * aPixelRatio,
                                             theEvent->position().y() * aPixelRatio));
#else
  const qreal aPixelRatio = devicePixelRatioF();
  const Graphic3d_Vec2i aPos(theEvent->pos().x() * aPixelRatio, theEvent->pos().y() * aPixelRatio);
#endif
  // If subviews present, redirect wheel focus to picked subview
  if (!m_view->Subviews().IsEmpty())
  {
    Handle(V3d_View) aPickedView = m_view->PickSubview(aPos);
    if (!aPickedView.IsNull() && aPickedView != m_focusView)
    {
      OnSubviewChanged(m_context, m_focusView, aPickedView);
      updateView();
      return;
    }
  }
  if (UpdateZoom(Aspect_ScrollDelta(aPos, double(theEvent->angleDelta().y()) / 8.0)))
  {
    updateGridStepForView(!m_focusView.IsNull() ? m_focusView : m_view);
    updateView();
  }
}

void OcctQOpenGLWidgetViewer::updateView()
{
  update();
}

void OcctQOpenGLWidgetViewer::paintGL()
{
  if (m_view.IsNull() || m_view->Window().IsNull()) return;
  Aspect_Drawable aNativeWin = (Aspect_Drawable)winId();
#ifdef _WIN32
  HDC  aWglDevCtx = wglGetCurrentDC();
  HWND aWglWin    = WindowFromDC(aWglDevCtx);
  aNativeWin      = (Aspect_Drawable)aWglWin;
#endif
  if (m_view->Window()->NativeHandle() != aNativeWin)
  {
    initializeGL();
    return;
  }
  Handle(OpenGl_Context)     aGlCtx      = OcctGlTools::GetGlContext(m_view);
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
  Handle(Aspect_NeutralWindow) aWindow = Handle(Aspect_NeutralWindow)::DownCast(m_view->Window());
  aWindow->Size(aViewSizeOld.x(), aViewSizeOld.y());
  if (aViewSizeNew != aViewSizeOld)
  {
    aWindow->SetSize(aViewSizeNew.x(), aViewSizeNew.y());
    m_view->MustBeResized();
    m_view->Invalidate();
    dumpGlInfo(true, false);
    for (const Handle(V3d_View)& aSubviewIter : m_view->Subviews())
    {
      aSubviewIter->MustBeResized();
      aSubviewIter->Invalidate();
      aDefaultFbo->SetupViewport(aGlCtx);
    }
  }
  Handle(V3d_View) aView = !m_focusView.IsNull() ? m_focusView : m_view;
  aView->InvalidateImmediate();
  FlushViewEvents(m_context, aView, true);
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
  if (theView.IsNull() || m_viewer.IsNull()) return;
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
  if (Abs(snapped - m_gridStep) < (1.0e-6 * Max(1.0, m_gridStep))) return;
  m_gridStep = snapped;
  m_viewer->SetRectangularGridValues(0.0, 0.0, m_gridStep, m_gridStep, 0.0);
  m_viewer->ActivateGrid(Aspect_GT_Rectangular, Aspect_GDM_Lines);
}

void OcctQOpenGLWidgetViewer::OnSubviewChanged(const Handle(AIS_InteractiveContext)&,
                                               const Handle(V3d_View)&,
                                               const Handle(V3d_View)& theNewView)
{
  m_focusView = theNewView;
}

Handle(AIS_Shape) OcctQOpenGLWidgetViewer::addBody(const TopoDS_Shape& theShape,
                                                   AIS_DisplayMode theDispMode,
                                                   Standard_Integer theDispPriority,
                                                   bool theToUpdate)
{
  Handle(AIS_Shape) aShape = new AIS_Shape(theShape);
  m_bodies.Append(aShape);
  m_context->Display(aShape, theDispMode, theDispPriority, theToUpdate);
  return aShape;
}

void OcctQOpenGLWidgetViewer::clearBodies(bool theToUpdate)
{
  for (NCollection_Sequence<Handle(AIS_Shape)>::Iterator it(m_bodies); it.More(); it.Next())
  {
    const Handle(AIS_Shape)& aBody = it.Value();
    if (!aBody.IsNull() && m_context->IsDisplayed(aBody))
    {
      m_context->Erase(aBody, false);
    }
  }
  m_bodies.Clear();
  if (theToUpdate)
  {
    if (!m_view.IsNull()) { m_context->UpdateCurrentViewer(); m_view->Invalidate(); }
    update();
  }
}

// setBodiesVisible / toggleBodiesVisible removed per UI simplification
Handle(AIS_Shape) OcctQOpenGLWidgetViewer::selectedShape() const
{
  Handle(AIS_Shape) result;
  for (m_context->InitSelected(); m_context->MoreSelected(); m_context->NextSelected())
  {
    result = Handle(AIS_Shape)::DownCast(m_context->SelectedInteractive());
    if (!result.IsNull()) break;
  }
  return result;
}

Handle(AIS_Shape) OcctQOpenGLWidgetViewer::detectedShape() const
{
  if (!m_context.IsNull() && m_context->HasDetected())
  {
    return Handle(AIS_Shape)::DownCast(m_context->DetectedInteractive());
  }
  return Handle(AIS_Shape)();
}
