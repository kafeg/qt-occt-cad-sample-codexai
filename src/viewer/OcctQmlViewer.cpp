#include "OcctQmlViewer.h"

#include <Standard_WarningsDisable.hxx>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QQuickWindow>
#include <QSurfaceFormat>
#include <Standard_WarningsRestore.hxx>

#include <OpenGl_Context.hxx>
#include <OpenGl_FrameBuffer.hxx>
#include <OpenGl_GraphicDriver.hxx>

#include <Aspect_DisplayConnection.hxx>
#include <Aspect_NeutralWindow.hxx>
#include <Graphic3d_Camera.hxx>
#include <Graphic3d_RenderingParams.hxx>
#include <Graphic3d_ZLayerSettings.hxx>
#include <Message.hxx>
#include <Prs3d_Drawer.hxx>
#include <Quantity_Color.hxx>
#include <TColStd_IndexedDataMapOfStringString.hxx>

#include <AIS_ViewCube.hxx>
#include "OcctGlTools.h"
#include "FiniteGrid.h"
#include "SceneGizmos.h"
#include "OcctQtTools.h"

namespace {
static void dumpGlInfoString(const Handle(V3d_View)& view, QString& out)
{
  if (view.IsNull()) return;
  TColStd_IndexedDataMapOfStringString dict;
  view->DiagnosticInformation(dict, Graphic3d_DiagnosticInfo_Basic);
  TCollection_AsciiString anInfo;
  for (TColStd_IndexedDataMapOfStringString::Iterator it(dict); it.More(); it.Next())
  {
    if (!it.Value().IsEmpty())
    {
      if (!anInfo.IsEmpty()) anInfo += "\n";
      anInfo += it.Key() + ": " + it.Value();
    }
  }
  out = QString::fromUtf8(anInfo.ToCString());
}
}

OcctQmlViewer::OcctQmlViewer(QQuickItem* parent)
  : QQuickFramebufferObject(parent)
{
  setFlag(ItemHasContents, true);
  setAcceptHoverEvents(true);
  setAcceptedMouseButtons(Qt::AllButtons);
  setFlag(ItemIsFocusScope, true);
  setFocus(true);
  // Ensure first frame gets scheduled even without external triggers
  update();
}

OcctQmlViewer::~OcctQmlViewer() = default;

QQuickFramebufferObject::Renderer* OcctQmlViewer::createRenderer() const
{
  return new RendererImpl();
}

void OcctQmlViewer::resetViewToOrigin(double distance)
{
  QMutexLocker lock(&m_mutex);
  m_resetRequested = true;
  m_resetDistance = distance;
  update();
}

void OcctQmlViewer::clearBodies()
{
  QMutexLocker lock(&m_mutex);
  m_clearRequested = true;
  update();
}

Handle(AIS_Shape) OcctQmlViewer::addShape(const TopoDS_Shape& theShape,
                                          AIS_DisplayMode    theDispMode,
                                          Standard_Integer   theDispPriority,
                                          bool               /*theToUpdate*/)
{
  Q_UNUSED(theDispPriority);
  PendingShape op;
  op.shape   = theShape;
  op.dispMode = theDispMode;
  {
    QMutexLocker lock(&m_mutex);
    m_pendingShapes.push_back(op);
  }
  update();
  return Handle(AIS_Shape)(); // actual AIS handle created on render thread
}

void OcctQmlViewer::setDatum(const std::shared_ptr<Datum>& d)
{
  QMutexLocker lock(&m_mutex);
  m_datum = d;
  update();
}

void OcctQmlViewer::updateGlInfoFromRenderer(const QString& info)
{
  if (m_glInfo == info)
    return;
  m_glInfo = info;
  emit glInfoChanged();
}

void OcctQmlViewer::pullInput(bool& rotStart, bool& rotActive, bool& panActive,
                              QPoint& currPos, QPoint& lastPos) const
{
  QMutexLocker lock(&m_mutex);
  rotStart  = m_rotStartPending; m_rotStartPending = false;
  rotActive = m_rotActive;
  panActive = m_panActive;
  currPos   = m_currPosPx;
  lastPos   = m_lastPosPx;
}

// ================= Input handling (main thread) =================

void OcctQmlViewer::keyPressEvent(QKeyEvent* e)
{
  QQuickFramebufferObject::keyPressEvent(e);
}

static inline qreal qmlDevicePixelRatio(const QQuickItem* it)
{
  if (auto* w = it->window()) return w->devicePixelRatio();
  return 1.0;
}

void OcctQmlViewer::mousePressEvent(QMouseEvent* e)
{
  const qreal pr = qmlDevicePixelRatio(this);
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
  const QPointF pt = e->position();
#else
  const QPointF pt = e->posF();
#endif
  const Graphic3d_Vec2i p(int(pt.x() * pr), int(pt.y() * pr));
  const Aspect_VKeyFlags mods = OcctQtTools::qtMouseModifiers2VKeys(e->modifiers());
  if (UpdateMouseButtons(p, OcctQtTools::qtMouseButtons2VKeys(e->buttons()), mods, false))
    update();
  {
    QMutexLocker lock(&m_mutex);
    m_currPosPx = m_lastPosPx = QPoint(int(pt.x() * pr), int(pt.y() * pr));
    m_pressPosPx = m_currPosPx;
    if (e->button() == Qt::RightButton || (e->button() == Qt::LeftButton && (mods & Aspect_VKeyFlags_ALT)))
    {
      m_rotActive = true;
      m_rotStartPending = true;
    }
    else if (e->button() == Qt::MiddleButton)
    {
      m_panActive = true;
    }
    else if (e->button() == Qt::LeftButton)
    {
      // Defer decision: if это был drag, включим орбиту; короткий клик оставим на selection/ViewCube
      m_leftDown = true;
      m_rotActive = false;
      m_rotStartPending = false;
    }
  }
  grabMouse();
  e->accept();
}

void OcctQmlViewer::mouseMoveEvent(QMouseEvent* e)
{
  const qreal pr = qmlDevicePixelRatio(this);
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
  const QPointF pt = e->position();
#else
  const QPointF pt = e->posF();
#endif
  const Graphic3d_Vec2i p(int(pt.x() * pr), int(pt.y() * pr));
  if (UpdateMousePosition(p,
                          OcctQtTools::qtMouseButtons2VKeys(e->buttons()),
                          OcctQtTools::qtMouseModifiers2VKeys(e->modifiers()),
                          false))
    update();
  {
    QMutexLocker lock(&m_mutex);
    m_currPosPx = QPoint(int(pt.x() * pr), int(pt.y() * pr));
    if (m_leftDown && !m_panActive && !m_rotActive)
    {
      const QPoint d = m_currPosPx - m_pressPosPx;
      if (std::abs(d.x()) > kDragThresholdPx || std::abs(d.y()) > kDragThresholdPx)
      {
        m_rotActive = true;
        m_rotStartPending = true;
      }
    }
  }
  e->accept();
}

void OcctQmlViewer::mouseReleaseEvent(QMouseEvent* e)
{
  const qreal pr = qmlDevicePixelRatio(this);
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
  const QPointF pt = e->position();
#else
  const QPointF pt = e->posF();
#endif
  const Graphic3d_Vec2i p(int(pt.x() * pr), int(pt.y() * pr));
  const Aspect_VKeyFlags mods = OcctQtTools::qtMouseModifiers2VKeys(e->modifiers());
  if (UpdateMouseButtons(p, OcctQtTools::qtMouseButtons2VKeys(e->buttons()), mods, false))
    update();
  {
    QMutexLocker lock(&m_mutex);
    m_lastPosPx = m_currPosPx = QPoint(int(pt.x() * pr), int(pt.y() * pr));
    m_rotActive = false;
    m_panActive = false;
    m_leftDown = false;
  }
  ungrabMouse();
  e->accept();
}

void OcctQmlViewer::wheelEvent(QWheelEvent* e)
{
  const qreal pr = qmlDevicePixelRatio(this);
  const Graphic3d_Vec2i p(int(e->position().x() * pr), int(e->position().y() * pr));
  // Normalize wheel delta to "steps" (120 per detent); fall back to pixelDelta if needed
  double steps = 0.0;
  if (!e->angleDelta().isNull()) steps = e->angleDelta().y() / 120.0;
  else if (!e->pixelDelta().isNull()) steps = e->pixelDelta().y() / 60.0;
  if (UpdateZoom(Aspect_ScrollDelta(p, steps)))
    update();
  e->accept();
}

void OcctQmlViewer::takePending(std::vector<PendingShape>& outShapes,
                                bool& outDoClear,
                                bool& outDoReset,
                                double& outResetDist,
                                std::shared_ptr<Datum>& outDatum,
                                QString& outGlInfo) const
{
  QMutexLocker lock(&m_mutex);
  outShapes.swap(const_cast<std::vector<PendingShape>&>(m_pendingShapes));
  outDoClear            = m_clearRequested;  const_cast<bool&>(m_clearRequested) = false;
  outDoReset            = m_resetRequested;  const_cast<bool&>(m_resetRequested) = false;
  outResetDist          = m_resetDistance;
  outDatum              = m_datum;
  outGlInfo             = m_glInfo;
}

// ================= RendererImpl =================

OcctQmlViewer::RendererImpl::RendererImpl() = default;
OcctQmlViewer::RendererImpl::~RendererImpl() = default;

QOpenGLFramebufferObject* OcctQmlViewer::RendererImpl::createFramebufferObject(const QSize& size)
{
  QOpenGLFramebufferObjectFormat fmt;
  // Use combined depth-stencil for broadest compatibility; avoid MSAA here
  // since some RHI/driver combos with Qt Quick + CoreProfile ignore it.
  fmt.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
  return new QOpenGLFramebufferObject(size, fmt);
}

void OcctQmlViewer::RendererImpl::synchronize(QQuickFramebufferObject* item)
{
  auto* v = static_cast<OcctQmlViewer*>(item);
  v->takePending(m_toAdd, m_doClear, m_doReset, m_resetDistance, m_datum, m_glInfo);
  // Push GL info gathered on the render thread back to the item for QML binding
  v->updateGlInfoFromRenderer(m_glInfo);
  m_owner = v;
}

void OcctQmlViewer::RendererImpl::ensureOcctContext()
{
  if (!m_viewer.IsNull()) return;

  Handle(Aspect_DisplayConnection) aDisp   = new Aspect_DisplayConnection();
  Handle(OpenGl_GraphicDriver)     aDriver = new OpenGl_GraphicDriver(aDisp, false);
  aDriver->ChangeOptions().buffersNoSwap = true;
  aDriver->ChangeOptions().buffersOpaqueAlpha = true;
  aDriver->ChangeOptions().useSystemBuffer = false;

  m_viewer = new V3d_Viewer(aDriver);
  const Quantity_Color aBgTop(1.0, 1.0, 1.0, Quantity_TOC_sRGB);
  const Quantity_Color aBgBottom(0.92, 0.92, 0.92, Quantity_TOC_sRGB);
  m_viewer->SetDefaultBackgroundColor(aBgTop);
  m_viewer->SetDefaultBgGradientColors(aBgTop, aBgBottom, Aspect_GradientFillMethod_Elliptical);
  m_viewer->SetDefaultLights();
  m_viewer->SetLightOn();

  m_context = new AIS_InteractiveContext(m_viewer);

  m_view = m_viewer->CreateView();
  m_view->SetBgGradientColors(aBgTop, aBgBottom, Aspect_GradientFillMethod_Elliptical);
  m_view->SetImmediateUpdate(false);
  m_view->ChangeRenderingParams().ToShowStats = false;
  m_view->ChangeRenderingParams().StatsTextHeight = 24;
  m_view->ChangeRenderingParams().CollectedStats = (Graphic3d_RenderingParams::PerfCounters)(
    Graphic3d_RenderingParams::PerfCounters_FrameRate | Graphic3d_RenderingParams::PerfCounters_Triangles);

  // Bind OCCT to current GL context and our FBO-backed window
  Handle(OpenGl_Context) aGlCtx = new OpenGl_Context();
  if (!aGlCtx->Init(true))
  {
    Message::SendFail() << "OpenGl_Context init failed in QML viewer";
    return;
  }

  Handle(Aspect_NeutralWindow) aWindow = new Aspect_NeutralWindow();
  aWindow->SetVirtual(true);
  QSize s = framebufferObject() ? framebufferObject()->size() : QSize(16, 16);
  aWindow->SetSize(s.width(), s.height());
  m_view->SetWindow(aWindow, aGlCtx->RenderingContext());

  // Configure Topmost as overlay (no depth)
  Graphic3d_ZLayerSettings topmost = m_viewer->ZLayerSettings(Graphic3d_ZLayerId_Topmost);
  topmost.SetEnableDepthTest(Standard_False);
  topmost.SetEnableDepthWrite(Standard_False);
  topmost.SetClearDepth(Standard_False);
  m_viewer->SetZLayerSettings(Graphic3d_ZLayerId_Topmost, topmost);

  initViewDefaults();

  // Populate GL info once
  dumpGlInfoString(m_view, m_glInfo);

  // Minimal on-screen gizmo so the user sees the viewer is alive
  if (m_viewCube.IsNull())
  {
    m_viewCube = new AIS_ViewCube();
    m_viewCube->SetFixedAnimationLoop(false);
    m_viewCube->SetAutoStartAnimation(true);
    m_viewCube->SetSize(60.0);
    m_viewCube->SetBoxColor(Quantity_NOC_GRAY70);
    m_viewCube->TransformPersistence()->SetCorner2d(Aspect_TOTP_RIGHT_UPPER);
    m_viewCube->TransformPersistence()->SetOffset2d(Graphic3d_Vec2i(80, 80));
    if (!m_context.IsNull())
    {
      m_context->Display(m_viewCube, 0, -1, false);
      m_context->SetZLayer(m_viewCube, Graphic3d_ZLayerId_Topmost);
    }
  }

  // Display a simple finite grid and gizmos if datum is provided
  if (m_grid.IsNull())
  {
    m_grid = new FiniteGrid();
    if (!m_context.IsNull())
    {
      m_context->Display(m_grid, Standard_False);
      m_context->SetZLayer(m_grid, Graphic3d_ZLayerId_Default);
    }
  }
}

void OcctQmlViewer::RendererImpl::initViewDefaults()
{
  if (m_view.IsNull()) return;
  // Basic camera default similar to widget viewer
  resetViewToOriginInternal(1.2);
}

void OcctQmlViewer::RendererImpl::resetViewToOriginInternal(double distance)
{
  if (m_view.IsNull()) return;
  if (!(distance > 0.0)) distance = 5.0;
  m_view->SetAt(0.0, 0.0, 0.0);
  m_view->SetProj(V3d_XposYnegZpos);
  m_view->SetEye(distance, -distance, distance);
  Standard_Real aW = 0.0, aH = 0.0;
  m_view->Size(aW, aH);
  const Standard_Real aMax = (aW > aH ? aW : aH);
  if (aMax > 0.0)
  {
    m_view->SetSize(aMax * 0.20);
  }
  m_view->SetScale(3.000);
  if (!m_context.IsNull()) m_context->UpdateCurrentViewer();
  m_view->Invalidate();
}

void OcctQmlViewer::RendererImpl::updateWindowSizeFromFbo()
{
  if (m_view.IsNull()) return;
  Handle(Aspect_NeutralWindow) aWindow = Handle(Aspect_NeutralWindow)::DownCast(m_view->Window());
  if (aWindow.IsNull()) return;
  QSize s = framebufferObject() ? framebufferObject()->size() : QSize(16, 16);
  Standard_Integer w = 0, h = 0;
  aWindow->Size(w, h);
  if (w != s.width() || h != s.height())
  {
    aWindow->SetSize(s.width(), s.height());
    m_view->MustBeResized();
    m_view->Invalidate();
  }
}

void OcctQmlViewer::RendererImpl::applyPending()
{
  if (m_view.IsNull() || m_context.IsNull())
  {
    m_toAdd.clear();
    m_doClear = false;
    m_doReset = false;
    return;
  }

  if (m_doClear)
  {
    m_context->RemoveAll(false);
    m_doClear = false;
  }
  // Ensure gizmos follow datum
  if (m_datum)
  {
    if (!m_gizmos) m_gizmos = std::make_unique<SceneGizmos>();
    m_gizmos->erase(m_context);
    m_gizmos->install(m_context, m_datum, Standard_True);
  }
  for (const PendingShape& op : m_toAdd)
  {
    Handle(AIS_Shape) ais = new AIS_Shape(op.shape);
    m_context->Display(ais, op.dispMode, 0, false);
  }
  m_toAdd.clear();

  if (m_doReset)
  {
    resetViewToOriginInternal(m_resetDistance);
    m_doReset = false;
  }
}

void OcctQmlViewer::RendererImpl::render()
{
  ensureOcctContext();

  // Wrap current Qt FBO as OCCT default using the driver's shared GL context
  if (!m_view.IsNull())
  {
    Handle(OpenGl_Context) aGlCtx = OcctGlTools::GetGlContext(m_view);
    if (!aGlCtx.IsNull())
    {
      Handle(OpenGl_FrameBuffer) aDefaultFbo = aGlCtx->DefaultFrameBuffer();
      if (aDefaultFbo.IsNull())
      {
        aDefaultFbo = new OpenGl_FrameBuffer();
        aGlCtx->SetDefaultFrameBuffer(aDefaultFbo);
      }
      if (aDefaultFbo->InitWrapper(aGlCtx))
      {
        // Keep OCCT view size in sync with Qt FBO
        updateWindowSizeFromFbo();
      }
    }
  }
  // Apply any pending view controller events from UI thread BEFORE drawing
  if (m_owner)
  {
    Handle(V3d_View) aView = m_view;
    m_owner->FlushViewEvents(m_context, aView, true);
    // Fallback manual navigation to match widget UX
    bool rotStart=false, rotActive=false, panActive=false; QPoint curr, last;
    m_owner->pullInput(rotStart, rotActive, panActive, curr, last);
    if (!m_view.IsNull())
    {
      if (rotStart)
      {
        m_view->StartRotation(curr.x(), curr.y());
      }
      if (rotActive)
      {
        m_view->Rotation(curr.x(), curr.y());
      }
      else if (panActive)
      {
        const int dx = curr.x() - last.x();
        const int dy = curr.y() - last.y();
        if (dx != 0 || dy != 0)
          m_view->Pan(dx, dy);
      }
    }
  }

  applyPending();

  if (!m_view.IsNull())
  {
    // Visual fallback: clear to light gray to prove FBO is visible
    if (QOpenGLContext* ctx = QOpenGLContext::currentContext())
    {
      QOpenGLFunctions* f = ctx->functions();
      f->glViewport(0, 0, framebufferObject()->width(), framebufferObject()->height());
      f->glClearColor(0.95f, 0.95f, 0.95f, 1.0f);
      f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    m_view->InvalidateImmediate();
    // Keep grid in sync with current view extents
    if (!m_grid.IsNull())
    {
      Handle(FiniteGrid) grid = Handle(FiniteGrid)::DownCast(m_grid);
      if (!grid.IsNull())
      {
        grid->updateFromView(m_view);
        m_context->Redisplay(grid, Standard_False);
      }
    }
    if (!m_context.IsNull()) m_context->UpdateCurrentViewer();
    m_view->Redraw();
  }

  update(); // request next frame if needed
}
