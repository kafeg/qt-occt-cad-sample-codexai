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
  fmt.setAttachment(QOpenGLFramebufferObject::Depth);
  fmt.setSamples(4);
  return new QOpenGLFramebufferObject(size, fmt);
}

void OcctQmlViewer::RendererImpl::synchronize(QQuickFramebufferObject* item)
{
  auto* v = static_cast<OcctQmlViewer*>(item);
  v->takePending(m_toAdd, m_doClear, m_doReset, m_resetDistance, m_datum, m_glInfo);
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

  // Wrap current FBO as OCCT default
  Handle(OpenGl_Context) aGlCtx = new OpenGl_Context();
  if (aGlCtx->Init(true))
  {
    Handle(OpenGl_FrameBuffer) aDefaultFbo = aGlCtx->DefaultFrameBuffer();
    if (aDefaultFbo.IsNull())
    {
      aDefaultFbo = new OpenGl_FrameBuffer();
      aGlCtx->SetDefaultFrameBuffer(aDefaultFbo);
    }
    aDefaultFbo->InitWrapper(aGlCtx);
  }

  updateWindowSizeFromFbo();
  applyPending();

  if (!m_view.IsNull())
  {
    m_view->InvalidateImmediate();
    if (!m_context.IsNull()) m_context->UpdateCurrentViewer();
    m_view->Redraw();
  }

  update(); // request next frame if needed
}

