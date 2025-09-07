#include "OcctQOpenGLWidgetViewer.h"
#include "OcctGlTools.h"
#include "OcctQtTools.h"

#include <Standard_WarningsDisable.hxx>
#include <QApplication>
#include <QMessageBox>
#include <QMouseEvent>
#include <QSurfaceFormat>
#include <Standard_WarningsRestore.hxx>

#include <QResource>
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
#include <QPainter>
#include <Standard_Version.hxx>
#include <PrsMgr_DisplayStatus.hxx>
#include <Graphic3d_ZLayerSettings.hxx>
#include <Graphic3d_DisplayPriority.hxx>
#include <Graphic3d_NameOfMaterial.hxx>
#include "FiniteGrid.h"
#include <gp_Pnt.hxx>
#include "SceneGizmos.h"
#include "CustomManipulator.h"

#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Wire.hxx>

#include <Sketch.h>
#include <Datum.h>
#include <TColStd_IndexedDataMapOfStringString.hxx>
#include <TCollection_AsciiString.hxx>

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
  // Ensure resources from viewer.qrc are registered (static library case)
  Q_INIT_RESOURCE(viewer);

  Handle(Aspect_DisplayConnection) aDisp   = new Aspect_DisplayConnection();
  Handle(OpenGl_GraphicDriver)     aDriver = new OpenGl_GraphicDriver(aDisp, false);
  aDriver->ChangeOptions().buffersNoSwap = true;
  aDriver->ChangeOptions().buffersOpaqueAlpha = true;
  aDriver->ChangeOptions().useSystemBuffer = false;

  m_viewer = new V3d_Viewer(aDriver);
  // Background: subtle elliptical gradient (white center to light gray edges)
  const Quantity_Color aBgTop(1.0, 1.0, 1.0, Quantity_TOC_sRGB);      // center
  const Quantity_Color aBgBottom(0.92, 0.92, 0.92, Quantity_TOC_sRGB); // edges (stronger)
  m_viewer->SetDefaultBackgroundColor(aBgTop);
  m_viewer->SetDefaultBgGradientColors(aBgTop, aBgBottom, Aspect_GradientFillMethod_Elliptical);
  m_viewer->SetDefaultLights();
  m_viewer->SetLightOn();
  // No built-in OCCT grid; a custom AIS grid object is used instead

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
  // Disable built-in on-screen stats; we display them in a side panel
  m_view->ChangeRenderingParams().ToShowStats = false;
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

void OcctQOpenGLWidgetViewer::resetViewToOrigin(Standard_Real distance)
{
  if (m_view.IsNull()) return;

  if (!(distance > 0.0)) distance = 5.0;

  // Ignore previous distance/orientation; set fixed orientation and eye
  m_view->SetAt(0.0, 0.0, 0.0);
  m_view->SetProj(V3d_XposYnegZpos);
  // Place eye along the isometric (+X, -Y, +Z) direction
  m_view->SetEye(distance, -distance, distance);

  // Make the initial framing closer by shrinking the view size (orthographic zoom).
  // We scale current view size down to 20% to appear ~5x closer by default.
  Standard_Real aW = 0.0, aH = 0.0;
  m_view->Size(aW, aH);
  const Standard_Real aMax = (aW > aH ? aW : aH);
  if (aMax > 0.0)
  {
    m_view->SetSize(aMax * 0.20); // smaller size => larger zoom-in
  }

  // Invalidate and request repaint; also update grid placement if present
  if (!m_context.IsNull())
  {
    m_context->UpdateCurrentViewer();
  }
  m_view->Invalidate();
  if (!m_grid.IsNull())
  {
    Handle(FiniteGrid) grid = Handle(FiniteGrid)::DownCast(m_grid);
    if (!grid.IsNull()) { grid->updateFromView(m_view); m_context->Redisplay(grid, Standard_False); }
  }
  // Apply a consistent initial zoom scale for readability in stats
  // Previous default hovered around ~5.000; cut it to 3.000 as requested
  m_view->SetScale(3.000);
  update();
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
    // Display view cube using its default mode
    m_context->Display(m_viewCube, Standard_False);
  }

  // Configure Z-layer behavior: make Topmost a pure overlay (no depth test/write)
  {
    Graphic3d_ZLayerSettings topmost = m_viewer->ZLayerSettings(Graphic3d_ZLayerId_Topmost);
    topmost.SetName("TopmostOverlay");
    topmost.SetEnableDepthTest(Standard_False);
    topmost.SetEnableDepthWrite(Standard_False);
    // Do not clear depth; keep previous layers' depth for other overlays that may inherit
    topmost.SetClearDepth(Standard_False);
    m_viewer->SetZLayerSettings(Graphic3d_ZLayerId_Topmost, topmost);
  }

  // Create custom Z-layers to control order: Default < Axes < Sketch < Top
  {
    Graphic3d_ZLayerSettings axes;
    axes.SetName("AxesLayer");
    axes.SetEnableDepthTest(Standard_True);
    axes.SetEnableDepthWrite(Standard_True);
    // Insert Axes before Sketch (which we insert before Top)
    Graphic3d_ZLayerSettings sketch;
    sketch.SetName("SketchLayer");
    sketch.SetEnableDepthTest(Standard_True);
    sketch.SetEnableDepthWrite(Standard_True);

    // First insert Sketch before Top to have it between Default and Top
    m_viewer->InsertLayerBefore(m_layerSketch, sketch, Graphic3d_ZLayerId_Top);
    // Then insert Axes before Sketch so that Default < Axes < Sketch < Top
    m_viewer->InsertLayerBefore(m_layerAxes, axes, m_layerSketch);
  }

  {
    // Gizmos overlay: axes + planes driven by Document Datum
    if (!m_gizmos) m_gizmos = std::make_unique<SceneGizmos>();
    if (m_datum)
    {
      m_gizmos->install(m_context, m_datum, Standard_True);
      if (m_gizmos)
      {
        if (!m_gizmos->bgAxisX().IsNull()) m_context->SetZLayer(m_gizmos->bgAxisX(), Graphic3d_ZLayerId_Default);
        if (!m_gizmos->bgAxisY().IsNull()) m_context->SetZLayer(m_gizmos->bgAxisY(), Graphic3d_ZLayerId_Default);
        if (!m_gizmos->trihedron().IsNull()) m_context->SetZLayer(m_gizmos->trihedron(), m_layerAxes);
        // Apply per-axis visibility from Datum
        if (m_datum)
          m_gizmos->setTrihedronAxesVisibility(m_context,
                                               m_datum->showTrihedronAxisX(),
                                               m_datum->showTrihedronAxisY(),
                                               m_datum->showTrihedronAxisZ());
      }
    }
    // Display custom infinite grid and initialize (force immediate update so it becomes visible)
    m_grid = new FiniteGrid();
    // Grid presentation; selection is disabled via empty ComputeSelection()
    m_context->Display(m_grid, 0, 0, true, PrsMgr_DisplayStatus_Displayed);
    // Keep grid in Default layer explicitly (depth-aware, behind overlays)
    m_context->SetZLayer(m_grid, Graphic3d_ZLayerId_Default);
    Handle(FiniteGrid) grid = Handle(FiniteGrid)::DownCast(m_grid);
    if (!grid.IsNull()) {
      grid->updateFromView(m_view);
      m_context->Redisplay(grid, Standard_False);
      // Gizmos are zoom-pers and independent from grid extents
    }
  }

  // Position camera towards origin after grid and axes are displayed
  // Use a closer fixed distance from origin for better default framing
  resetViewToOrigin(1.2);
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
      const bool hadTri = !m_gizmos || !m_gizmos->trihedron().IsNull();
      if (m_gizmos) m_gizmos->erase(m_context);
      m_view->FitAll(0.01, false);
      if (m_gizmos && hadTri) m_gizmos->reinstall(m_context);
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
  // Only start manipulator transform on explicit left-button press while gizmo is detected under cursor
  // Activate the mode by selecting the detected gizmo part, then start transforming
  if (!m_manip.IsNull() && theEvent->button() == Qt::LeftButton
      && !m_context.IsNull() && m_context->HasDetected()
      && m_context->DetectedInteractive() == m_manip)
  {
    m_context->SelectDetected(AIS_SelectionScheme_Replace);
    if (m_manip->HasActiveMode())
    {
      m_isManipDragging = true;
      m_lastManipDelta = gp_Trsf();
      m_manip->StartTransform(aPnt.x(), aPnt.y(), m_view);
      update();
      return;
    }
  }
  if (UpdateMouseButtons(aPnt, OcctQtTools::qtMouseButtons2VKeys(theEvent->buttons()), aFlags, false))
    updateView();
}

void OcctQOpenGLWidgetViewer::mouseReleaseEvent(QMouseEvent* theEvent)
{
  QOpenGLWidget::mouseReleaseEvent(theEvent);
  if (m_view.IsNull()) return;
  // If manipulator was dragging, finish manipulation and consume release
  if (!m_manip.IsNull() && m_isManipDragging)
  {
    m_manip->StopTransform(true);
    // Accumulate delta for multi-step manipulation in correct order
    // (new incremental delta applied before previously accumulated transform)
    m_manipAccumTrsf.PreMultiply(m_lastManipDelta);
    m_isManipDragging = false;
    // Deactivate current manipulator mode so further moves don't stick to last handle
    m_manip->DeactivateCurrentMode();
    // keep manipulator visible for further drags
    if (!m_view.IsNull()) { m_context->UpdateCurrentViewer(); m_view->Invalidate(); }
    update();
    return;
  }
  // No accumulation outside of a drag session.
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

void OcctQOpenGLWidgetViewer::setDatum(const std::shared_ptr<Datum>& d)
{
  m_datum = d;
  if (!m_context.IsNull())
  {
    if (!m_gizmos) m_gizmos = std::make_unique<SceneGizmos>();
    // Erase previous gizmos to avoid duplicates and apply new visibility flags
    m_gizmos->erase(m_context);
    if (m_datum)
    {
      m_gizmos->install(m_context, m_datum, Standard_True);
      if (!m_gizmos->bgAxisX().IsNull()) m_context->SetZLayer(m_gizmos->bgAxisX(), Graphic3d_ZLayerId_Default);
      if (!m_gizmos->bgAxisY().IsNull()) m_context->SetZLayer(m_gizmos->bgAxisY(), Graphic3d_ZLayerId_Default);
      if (!m_gizmos->trihedron().IsNull()) m_context->SetZLayer(m_gizmos->trihedron(), m_layerAxes);
      // Apply per-axis visibility from Datum
      m_gizmos->setTrihedronAxesVisibility(m_context,
                                           m_datum->showTrihedronAxisX(),
                                           m_datum->showTrihedronAxisY(),
                                           m_datum->showTrihedronAxisZ());
      if (!m_view.IsNull()) { m_context->UpdateCurrentViewer(); m_view->Invalidate(); }
      update();
    }
  }
}

void OcctQOpenGLWidgetViewer::mouseMoveEvent(QMouseEvent* theEvent)
{
  QOpenGLWidget::mouseMoveEvent(theEvent);
  if (m_view.IsNull()) return;
  const qreal aPixelRatio = devicePixelRatioF();
  const Graphic3d_Vec2i aNewPos(theEvent->pos().x() * aPixelRatio, theEvent->pos().y() * aPixelRatio);
  if (!m_manip.IsNull() && m_isManipDragging)
  {
    // Drag manipulator only; block camera motions
    m_lastManipDelta = m_manip->Transform(aNewPos.x(), aNewPos.y(), m_view);
    update();
    return;
  }
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
  { updateView(); }
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
  // Ensure OCCT stats text remains legible on light backgrounds by drawing
  // a subtle translucent backdrop in the top-left corner under it.
  {
    const Graphic3d_RenderingParams& rp = m_view->RenderingParams();
    if (rp.ToShowStats)
    {
      const int textH   = std::max(12, (int)rp.StatsTextHeight);
      const int nLines  = 2; // FrameRate and Triangles are enabled
      const int pad     = 6;
      const int boxW    = 320;
      const int boxH    = nLines * textH + 2 * pad;

      QPainter painter(this);
      painter.setRenderHint(QPainter::Antialiasing, true);
      QColor shadow(0, 0, 0, 60); // translucent dark panel
      painter.setPen(Qt::NoPen);
      painter.setBrush(shadow);
      painter.drawRoundedRect(QRect(6, 6, boxW, boxH), 6, 6);
    }
  }
}

void OcctQOpenGLWidgetViewer::handleViewRedraw(const Handle(AIS_InteractiveContext)& theCtx,
                                               const Handle(V3d_View)& theView)
{
  AIS_ViewController::handleViewRedraw(theCtx, theView);
  // Keep custom grid in sync with current view
  Handle(FiniteGrid) grid = Handle(FiniteGrid)::DownCast(m_grid);
  if (!grid.IsNull()) {
    grid->updateFromView(theView);
    theCtx->Redisplay(grid, Standard_False);
    if (m_gizmos) { m_gizmos->setAxisExtents(theCtx, grid->halfSizeX(), grid->halfSizeY()); }
  }
  if (myToAskNextFrame) updateView();
}

double OcctQOpenGLWidgetViewer::currentFps() const
{
  if (m_view.IsNull()) return 0.0;
  TColStd_IndexedDataMapOfStringString dict;
  m_view->StatisticInformation(dict);
  for (int i = 1; i <= dict.Extent(); ++i)
  {
    if (dict.FindKey(i).IsEqual(TCollection_AsciiString("FrameRate")))
    {
      const TCollection_AsciiString& v = dict.FindFromIndex(i);
      try { return std::stod(v.ToCString()); } catch (...) { return 0.0; }
    }
  }
  return 0.0;
}

Standard_Integer OcctQOpenGLWidgetViewer::currentTriangles() const
{
  if (m_view.IsNull()) return -1;
  TColStd_IndexedDataMapOfStringString dict;
  m_view->StatisticInformation(dict);
  for (int i = 1; i <= dict.Extent(); ++i)
  {
    if (dict.FindKey(i).IsEqual(TCollection_AsciiString("Triangles")))
    {
      const TCollection_AsciiString& v = dict.FindFromIndex(i);
      try { return static_cast<Standard_Integer>(std::stoll(v.ToCString())); } catch (...) { return -1; }
    }
  }
  return -1;
}

double OcctQOpenGLWidgetViewer::currentScale() const
{
  if (m_view.IsNull()) return 0.0;
  return m_view->Scale();
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

// updateGridStepForView removed: now handled by InfiniteGrid

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
  const bool wasEmpty = m_bodies.IsEmpty();
  Handle(AIS_Shape) aShape = new AIS_Shape(theShape);
  m_bodies.Append(aShape);
  aShape->SetDisplayMode(theDispMode);
  m_context->Display(aShape, theDispMode, 0, theToUpdate, PrsMgr_DisplayStatus_Displayed);
  // Bodies drawn last among 3D content
  m_context->SetZLayer(aShape, Graphic3d_ZLayerId_Top);
  if (theDispPriority != 0) { m_context->SetDisplayPriority(aShape, theDispPriority); }
  // No auto-fit here; initial camera is configured once at initialization.
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

void OcctQOpenGLWidgetViewer::showManipulator(const Handle(AIS_Shape)& onShape)
{
  if (onShape.IsNull()) return;
  hideManipulator();
  m_manip = new CustomManipulator();
  m_manip->SetSkinMode(AIS_Manipulator::ManipulatorSkin_Shaded); // keep shaded, custom hides scale knobs
  // Attach to the object with modes disabled; we'll enable only needed ones explicitly
  AIS_Manipulator::OptionsForAttach opts;
  opts.SetEnableModes(Standard_False); // do not auto-activate all selection modes
  m_manip->Attach(onShape, opts);
  // Hide all scaling visuals (center + axes) — not supported in MoveFeature
  m_manip->SetPart(AIS_ManipulatorMode::AIS_MM_Scaling, Standard_False);
  m_manip->SetPart(0, AIS_ManipulatorMode::AIS_MM_Scaling, Standard_False);
  m_manip->SetPart(1, AIS_ManipulatorMode::AIS_MM_Scaling, Standard_False);
  m_manip->SetPart(2, AIS_ManipulatorMode::AIS_MM_Scaling, Standard_False);
  // Hide translation plane handles (squares between axes) — also not supported
  m_manip->SetPart(AIS_ManipulatorMode::AIS_MM_TranslationPlane, Standard_False);
  m_manip->SetPart(0, AIS_ManipulatorMode::AIS_MM_TranslationPlane, Standard_False);
  m_manip->SetPart(1, AIS_ManipulatorMode::AIS_MM_TranslationPlane, Standard_False);
  m_manip->SetPart(2, AIS_ManipulatorMode::AIS_MM_TranslationPlane, Standard_False);
  m_manip->EnableMode(AIS_ManipulatorMode::AIS_MM_Translation);
  m_manip->EnableMode(AIS_ManipulatorMode::AIS_MM_Rotation);
  // Do NOT auto-activate on hover; start only on explicit press
  m_manip->SetModeActivationOnDetection(Standard_False);
  m_context->Display(m_manip, Standard_False);
  // Enable only required manipulation modes (no scaling or translation-plane)
  m_manip->EnableMode(AIS_ManipulatorMode::AIS_MM_Translation);
  m_manip->EnableMode(AIS_ManipulatorMode::AIS_MM_Rotation);
  // Ensure manipulator is always on top of 3D (independent depth) and drawn last in its layer
  m_context->SetZLayer(m_manip, Graphic3d_ZLayerId_Topmost);
  m_context->SetDisplayPriority(m_manip, Graphic3d_DisplayPriority_Topmost);
  m_lastManipDelta = gp_Trsf();
  m_isManipDragging = false;
  m_manipAccumTrsf = gp_Trsf();
  if (!m_view.IsNull()) { m_context->UpdateCurrentViewer(); m_view->Invalidate(); }
  update();
}

void OcctQOpenGLWidgetViewer::hideManipulator()
{
  if (m_manip.IsNull()) return;
  if (!m_context.IsNull() && m_context->IsDisplayed(m_manip))
  {
    m_context->Erase(m_manip, Standard_False);
  }
  m_manip->Detach();
  m_manip.Nullify();
  if (!m_view.IsNull()) { m_context->UpdateCurrentViewer(); m_view->Invalidate(); }
  update();
}

void OcctQOpenGLWidgetViewer::confirmManipulator()
{
  if (m_manip.IsNull()) return;
  emit manipulatorFinished(m_manipAccumTrsf);
  hideManipulator();
}

void OcctQOpenGLWidgetViewer::cancelManipulator()
{
  hideManipulator();
}

Handle(AIS_Shape) OcctQOpenGLWidgetViewer::addSketch(const std::shared_ptr<Sketch>& sketch)
{
  if (!sketch) return Handle(AIS_Shape)();
  // Convert sketch wires and build a compound for a single AIS handle
  const std::vector<TopoDS_Wire> wires = sketch->toOcctWires(1.0e-7);
  if (wires.empty()) return Handle(AIS_Shape)();

  BRep_Builder builder;
  TopoDS_Compound comp;
  builder.MakeCompound(comp);
  // For each wire: try to build a planar face (closed contour) for filled display.
  // If face creation fails (open contour), keep the wire as-is for line rendering.
  int numFaces = 0;
  for (const auto& w : wires)
  {
    BRepBuilderAPI_MakeFace mf(w, Standard_True);
    if (mf.IsDone())
    {
      builder.Add(comp, mf.Face());
      ++numFaces;
    }
    else
    {
      builder.Add(comp, w);
    }
  }

  Handle(AIS_Shape) ais = new AIS_Shape(comp);
  // Fixed sketch colors: semi-transparent light blue fill, saturated blue contours
  const Quantity_Color kFillCol(0.55, 0.80, 1.0, Quantity_TOC_sRGB);   // light blue
  const Quantity_Color kEdgeCol(0.00, 0.45, 1.0, Quantity_TOC_sRGB);   // saturated blue
  const Standard_ShortReal kAlpha = 0.55f; // 0..1, higher = more transparent

  Handle(Prs3d_Drawer) drw = ais->Attributes(); if (drw.IsNull()) drw = new Prs3d_Drawer();
  // Shaded mode for filled faces; explicit shading aspect to control interior color and blending
  drw->SetDisplayMode(AIS_Shaded);
  Handle(Prs3d_ShadingAspect) shade = drw->ShadingAspect(); if (shade.IsNull()) shade = new Prs3d_ShadingAspect();
  shade->SetMaterial(Graphic3d_NOM_PLASTIC);
  shade->SetColor(kFillCol);
  shade->SetTransparency(kAlpha);
  // Ensure alpha blending is enabled for faces
  Handle(Graphic3d_AspectFillArea3d) fillAsp = shade->Aspect();
  fillAsp->SetAlphaMode(Graphic3d_AlphaMode_Blend);
  // Pull faces slightly forward to ensure boundary lines draw cleanly
  fillAsp->SetPolygonOffsets(Aspect_POM_Fill, 1.0f, 1.0f);
  drw->SetShadingAspect(shade);
  Handle(Prs3d_LineAspect) edgeAspect = new Prs3d_LineAspect(kEdgeCol, Aspect_TOL_SOLID, 2.0f);
  drw->SetLineAspect(edgeAspect);
  drw->SetWireAspect(edgeAspect);
  drw->SetFaceBoundaryDraw(Standard_True);
  drw->SetFaceBoundaryAspect(edgeAspect);
  ais->SetAttributes(drw);
  // Apply transparency at AIS level as well
  ais->SetTransparency(kAlpha);

  m_sketches.Append(ais);
  ais->SetDisplayMode(AIS_Shaded);
  m_context->Display(ais, AIS_Shaded, 0, false, PrsMgr_DisplayStatus_Displayed);
  // Default: depth-aware sketches in dedicated Sketch layer (above axes, below bodies)
  m_context->SetZLayer(ais, m_layerSketch);
  // Keep mapping id -> AIS for edit overlay toggling
  m_sketchById[sketch->id()] = ais;
  return ais;
}

void OcctQOpenGLWidgetViewer::clearSketches(bool theToUpdate)
{
  for (NCollection_Sequence<Handle(AIS_Shape)>::Iterator it(m_sketches); it.More(); it.Next())
  {
    const Handle(AIS_Shape)& aSk = it.Value();
    if (!aSk.IsNull() && m_context->IsDisplayed(aSk))
    {
      m_context->Erase(aSk, false);
    }
  }
  m_sketches.Clear();
  m_sketchById.clear();
  m_activeSketchId = 0;
  if (theToUpdate)
  {
    if (!m_view.IsNull()) { m_context->UpdateCurrentViewer(); m_view->Invalidate(); }
    update();
  }
}

bool OcctQOpenGLWidgetViewer::setSketchEditMode(std::uint64_t sketchId, bool enabled, bool theToUpdate)
{
  if (sketchId == 0) return false;
  // Demote previously active
  if (m_activeSketchId != 0 && (!enabled || m_activeSketchId != sketchId))
  {
    auto itPrev = m_sketchById.find(m_activeSketchId);
    if (itPrev != m_sketchById.end() && !itPrev->second.IsNull())
    {
      Handle(AIS_Shape) prev = itPrev->second;
      m_context->SetZLayer(prev, m_layerSketch);
      Handle(Prs3d_Drawer) drwPrev = prev->Attributes(); if (drwPrev.IsNull()) drwPrev = new Prs3d_Drawer();
      if (!drwPrev->LineAspect().IsNull()) { drwPrev->LineAspect()->SetWidth(2.0f); }
      if (!drwPrev->WireAspect().IsNull()) { drwPrev->WireAspect()->SetWidth(2.0f); }
      if (!drwPrev->FaceBoundaryAspect().IsNull()) { drwPrev->FaceBoundaryAspect()->SetWidth(2.0f); }
      // Preserve sketch fill transparency when leaving edit mode
      prev->SetAttributes(drwPrev);
      m_context->Redisplay(prev, Standard_False);
    }
    m_activeSketchId = 0;
  }

  if (!enabled)
  {
    if (theToUpdate && !m_view.IsNull()) { m_context->UpdateCurrentViewer(); m_view->Invalidate(); update(); }
    return true;
  }

  auto it = m_sketchById.find(sketchId);
  if (it == m_sketchById.end() || it->second.IsNull()) return false;

  Handle(AIS_Shape) ais = it->second;
  // Promote to overlay: Topmost without depth (configured in initializeGL)
  m_context->SetZLayer(ais, Graphic3d_ZLayerId_Topmost);
  // Emphasize style a bit in edit mode
  Handle(Prs3d_Drawer) drw = ais->Attributes(); if (drw.IsNull()) drw = new Prs3d_Drawer();
  if (!drw->LineAspect().IsNull()) { drw->LineAspect()->SetWidth(3.0f); }
  if (!drw->WireAspect().IsNull()) { drw->WireAspect()->SetWidth(3.0f); }
  if (!drw->FaceBoundaryAspect().IsNull()) { drw->FaceBoundaryAspect()->SetWidth(3.0f); }
  // Keep semi-transparent fill in edit mode
  ais->SetAttributes(drw);
  m_context->Redisplay(ais, Standard_False);
  m_activeSketchId = sketchId;
  if (theToUpdate && !m_view.IsNull()) { m_context->UpdateCurrentViewer(); m_view->Invalidate(); update(); }
  return true;
}
