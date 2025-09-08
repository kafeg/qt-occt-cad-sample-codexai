#include "TabPageModel.h"

#include <Document.h>
#include <Sketch.h>
#include <Datum.h>
#include <MoveFeature.h>
#include <PlaneFeature.h>
#include <AxeFeature.h>
#include <PointFeature.h>

#include <Standard_WarningsDisable.hxx>
#include <QObject>
#include <QtGlobal>
#include <Standard_WarningsRestore.hxx>

#include <OcctQOpenGLWidgetViewer.h>
#include <AIS_Shape.hxx>
#include <Quantity_Color.hxx>
#include <Graphic3d_TransformPers.hxx>
#include <Graphic3d_ZLayerId.hxx>
#include <gp_Quaternion.hxx>
#include <gp_EulerSequence.hxx>

#include <cmath>
#include <algorithm>
#include <vector>
#include <AppSettings.h>

TabPageModel::TabPageModel(QObject* parent)
  : QObject(parent)
  , m_doc(std::make_unique<Document>())
{
}

TabPageModel::~TabPageModel() = default;

Document& TabPageModel::doc()
{
  return *m_doc;
}

void TabPageModel::setViewerObject(QObject* viewerObject)
{
  m_viewerObj = viewerObject;
  // Initialize viewer with document's datum when available
  if (auto* v = qobject_cast<OcctQOpenGLWidgetViewer*>(m_viewerObj))
  {
    v->setDatum(m_doc->datum());
  }
}

QObject* TabPageModel::createViewer(QObject* uiParent)
{
  // Create with QWidget parent when possible to integrate into layouts
  QWidget* wparent = qobject_cast<QWidget*>(uiParent);
  auto* viewer = new OcctQOpenGLWidgetViewer(wparent);
  setViewerObject(viewer);
  return viewer;
}

void TabPageModel::syncViewerFromDoc(bool toUpdate)
{
  auto* v = qobject_cast<OcctQOpenGLWidgetViewer*>(m_viewerObj);
  if (v == nullptr) return;
  v->clearBodies(false);
  v->clearSketches(false);
  m_featureToBody.Clear();
  m_bodyToFeature.Clear();
  m_sketchToHandle.clear();

  // Bodies from features
  const auto& feats = m_doc->features();
  for (NCollection_Sequence<Handle(Feature)>::Iterator it(feats); it.More(); it.Next())
  {
    const Handle(Feature)& f = it.Value();
    if (f.IsNull() || f->isSuppressed()) continue;
    Handle(AIS_Shape) body = v->addShape(f->shape(), AIS_Shaded, 0, false);
    if (body.IsNull()) continue;
    body->SetAutoHilight(true);
    m_featureToBody.Add(f, body);
    m_bodyToFeature.Add(body, f);
  }

  // Sketches
  for (const auto& sk : m_doc->sketches())
  {
    if (!sk) continue;
    Handle(AIS_Shape) h = v->addSketch(sk);
    if (!h.IsNull())
    {
      m_sketchToHandle[sk->id()] = h;
    }
  }

  // Datum helpers: show plane, point and axes bodies with special styling
  const auto& items = m_doc->items();
  // Planes
  for (NCollection_Sequence<Handle(DocumentItem)>::Iterator it(items); it.More(); it.Next())
  {
    Handle(PlaneFeature) pf = Handle(PlaneFeature)::DownCast(it.Value());
    if (pf.IsNull() || pf->isSuppressed()) continue;
    Handle(AIS_Shape) body = v->addShape(pf->shape(), AIS_Shaded, 0, false);
    // Color code by name suffix X/Y/Z (names set in initializer)
    TCollection_AsciiString nm = pf->name();
    Quantity_Color col(0.85, 0.85, 0.85, Quantity_TOC_sRGB);
    if (nm.Search("Plane XY") != -1) col = Quantity_Color(0.90, 0.90, 0.90, Quantity_TOC_sRGB);
    else if (nm.Search("Plane XZ") != -1) col = Quantity_Color(0.90, 0.90, 0.90, Quantity_TOC_sRGB);
    else if (nm.Search("Plane YZ") != -1) col = Quantity_Color(0.90, 0.90, 0.90, Quantity_TOC_sRGB);
    body->SetColor(col);
    if (!v->Context().IsNull())
    {
      v->Context()->SetZLayer(body, Graphic3d_ZLayerId_Top);
    }
    body->SetTransformPersistence(new Graphic3d_TransformPers(Graphic3d_TMF_ZoomPers, gp::Origin()));
    // Keep auto-highlight enabled for proper selection feedback
    body->SetAutoHilight(true);
    m_featureToBody.Add(pf, body);
    m_bodyToFeature.Add(body, pf);
  }
  // Origin point
  for (NCollection_Sequence<Handle(DocumentItem)>::Iterator it(items); it.More(); it.Next())
  {
    Handle(PointFeature) pt = Handle(PointFeature)::DownCast(it.Value());
    if (pt.IsNull() || pt->isSuppressed()) continue;
    Handle(AIS_Shape) body = v->addShape(pt->shape(), AIS_Shaded, 0, false);
    // Subtle, but visible yellow-ish point
    body->SetColor(Quantity_Color(1.0, 0.85, 0.25, Quantity_TOC_sRGB));
    if (!v->Context().IsNull())
    {
      v->Context()->SetZLayer(body, Graphic3d_ZLayerId_Top);
    }
    body->SetTransformPersistence(new Graphic3d_TransformPers(Graphic3d_TMF_ZoomPers, gp::Origin()));
    body->SetAutoHilight(true);
    m_featureToBody.Add(pt, body);
    m_bodyToFeature.Add(body, pt);
  }
  // Axes
  for (NCollection_Sequence<Handle(DocumentItem)>::Iterator it(items); it.More(); it.Next())
  {
    Handle(AxeFeature) ax = Handle(AxeFeature)::DownCast(it.Value());
    if (ax.IsNull() || ax->isSuppressed()) continue;
    Handle(AIS_Shape) body = v->addShape(ax->shape(), AIS_Shaded, 0, false);
    TCollection_AsciiString nm = ax->name();
    Quantity_Color col(0.85, 0.85, 0.85, Quantity_TOC_sRGB);
    if (nm.Search("Axis X") != -1) col = Quantity_Color(1.00, 0.20, 0.20, Quantity_TOC_sRGB);
    else if (nm.Search("Axis Y") != -1) col = Quantity_Color(0.25, 0.90, 0.25, Quantity_TOC_sRGB);
    else if (nm.Search("Axis Z") != -1) col = Quantity_Color(0.25, 0.45, 1.00, Quantity_TOC_sRGB);
    body->SetColor(col);
    if (!v->Context().IsNull())
    {
      v->Context()->SetZLayer(body, Graphic3d_ZLayerId_Top);
    }
    body->SetTransformPersistence(new Graphic3d_TransformPers(Graphic3d_TMF_ZoomPers, gp::Origin()));
    body->SetAutoHilight(true);
    m_featureToBody.Add(ax, body);
    m_bodyToFeature.Add(body, ax);
  }

  if (toUpdate)
  {
    if (!v->Context().IsNull()) v->Context()->UpdateCurrentViewer();
    v->View()->Invalidate();
    v->update();
  }
}

void TabPageModel::refreshFeatureList()
{
  emit featureListChanged();
  emit documentTreeChanged();
}

void TabPageModel::refreshDocumentTree()
{
  emit documentTreeChanged();
}

void TabPageModel::selectFeatureInViewer(const Handle(Feature)& f)
{
  auto* v = qobject_cast<OcctQOpenGLWidgetViewer*>(m_viewerObj);
  if (f.IsNull() || v == nullptr) return;
  if (!m_featureToBody.Contains(f)) return;
  Handle(AIS_Shape) body = Handle(AIS_Shape)::DownCast(m_featureToBody.FindFromKey(f));
  if (body.IsNull()) return;
  auto ctx = v->Context();
  ctx->ClearSelected(false);
  ctx->AddOrRemoveSelected(body, false);
  if (!v->Context().IsNull()) v->Context()->UpdateCurrentViewer();
  v->View()->Invalidate();
  v->update();
}

void TabPageModel::activateMove()
{
  auto* v = qobject_cast<OcctQOpenGLWidgetViewer*>(m_viewerObj);
  if (!v) return;
  Handle(AIS_Shape) sel = v->selectedShape();
  if (sel.IsNull()) return;
  if (!m_bodyToFeature.Contains(sel)) return;
  Handle(Feature) src = Handle(Feature)::DownCast(m_bodyToFeature.FindFromKey(sel));
  if (src.IsNull()) return;
  // Prevent moving fixed planes
  if (Handle(PlaneFeature) pf = Handle(PlaneFeature)::DownCast(src); !pf.IsNull() && pf->isFixedGeometry())
  {
    return;
  }
  // Prevent moving fixed origin point
  if (Handle(PointFeature) pt = Handle(PointFeature)::DownCast(src); !pt.IsNull() && pt->isFixedGeometry())
  {
    return;
  }

  v->showManipulator(sel);
  QObject::disconnect(v, &OcctQOpenGLWidgetViewer::manipulatorFinished, this, nullptr);
  if (m_connManipFinished) { QObject::disconnect(m_connManipFinished); m_connManipFinished = QMetaObject::Connection(); }
  m_connManipFinished = QObject::connect(v, &OcctQOpenGLWidgetViewer::manipulatorFinished, this, [this, src](const gp_Trsf& tr) {
    gp_XYZ t = tr.TranslationPart();
    gp_Quaternion q = tr.GetRotation();
    Standard_Real ax = 0.0, ay = 0.0, az = 0.0;
    q.GetEulerAngles(gp_Intrinsic_XYZ, ax, ay, az);
    const double r2d = 180.0 / M_PI;
    const double rx = ax * r2d;
    const double ry = ay * r2d;
    const double rz = az * r2d;

    Handle(MoveFeature) mf = new MoveFeature();
    mf->setSourceId(src->id());
    mf->setDeltaTrsf(tr);
    mf->setTranslation(t.X(), t.Y(), t.Z());
    mf->setRotation(rx, ry, rz);
    src->setSuppressed(true);
    m_doc->addFeature(mf);
    m_doc->recompute();
    syncViewerFromDoc(true);
    refreshFeatureList();
    if (m_connManipFinished) { QObject::disconnect(m_connManipFinished); m_connManipFinished = QMetaObject::Connection(); }
  });
}

void TabPageModel::confirmMove()
{
  auto* v = qobject_cast<OcctQOpenGLWidgetViewer*>(m_viewerObj);
  if (!v || !v->isManipulatorActive()) return;
  v->confirmManipulator();
  if (m_connManipFinished) { QObject::disconnect(m_connManipFinished); m_connManipFinished = QMetaObject::Connection(); }
}

void TabPageModel::cancelMove()
{
  auto* v = qobject_cast<OcctQOpenGLWidgetViewer*>(m_viewerObj);
  if (!v || !v->isManipulatorActive()) return;
  if (m_connManipFinished) { QObject::disconnect(m_connManipFinished); m_connManipFinished = QMetaObject::Connection(); }
  v->cancelManipulator();
  syncViewerFromDoc(true);
}
