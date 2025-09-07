#include "TabPage.h"

#include <OcctQOpenGLWidgetViewer.h>
#include "FeatureHistoryPanel.h"
#include "DocumentTreePanel.h"

#include <Standard_WarningsDisable.hxx>
#include <QVBoxLayout>
#include <QSplitter>
#include <Standard_WarningsRestore.hxx>

#include <Document.h>
#include <Sketch.h>
#include <AIS_Shape.hxx>
#include <MoveFeature.h>
#include <gp_Quaternion.hxx>
#include <gp_EulerSequence.hxx>
#include <cmath>
#include <algorithm>
#include <vector>

TabPage::TabPage(QWidget* parent)
  : QWidget(parent)
{
  auto* lay = new QVBoxLayout(this);
  lay->setContentsMargins(0, 0, 0, 0);
  auto* split = new QSplitter(Qt::Horizontal, this);
  split->setChildrenCollapsible(false);
  // Left sidebar: vertical splitter with History over Document tree
  QSplitter* left = new QSplitter(Qt::Vertical, split);
  left->setChildrenCollapsible(false);
  m_treePanel = new DocumentTreePanel(this, left);
  m_history = new FeatureHistoryPanel(this, left);
  left->addWidget(m_treePanel);   // document tree on top
  left->addWidget(m_history);     // history under it
  left->setStretchFactor(0, 1);
  left->setStretchFactor(1, 1);
  m_viewer = new OcctQOpenGLWidgetViewer(split);
  split->addWidget(left);
  split->addWidget(m_viewer);
  split->setStretchFactor(0, 0);
  split->setStretchFactor(1, 1);
  split->setSizes({260, 800});
  lay->addWidget(split);
  m_doc = std::make_unique<Document>();
  // Initialize viewer with document's Datum so gizmos render from it
  if (m_viewer && m_doc) { m_viewer->setDatum(m_doc->datum()); }

  // Connect panel actions
  connect(m_history, &FeatureHistoryPanel::requestRemoveSelected, [this]() {
    // Remove selected items; currently only Feature removal is supported
    auto sel = m_history->selectedItems();
    // Track sources of removed MoveFeatures to restore suppression when removing the last move
    std::vector<DocumentItem::Id> toUnsuppressIds;
    for (NCollection_Sequence<Handle(DocumentItem)>::Iterator it(sel); it.More(); it.Next())
    {
      const Handle(DocumentItem)& di = it.Value();
      if (Handle(Feature) f = Handle(Feature)::DownCast(di); !f.IsNull())
      {
        if (Handle(MoveFeature) mf = Handle(MoveFeature)::DownCast(f); !mf.IsNull())
        {
          if (mf->sourceId() != 0) toUnsuppressIds.push_back(mf->sourceId());
        }
        m_doc->removeFeature(f);
      }
      else if (!di.IsNull() && di->kind() == DocumentItem::Kind::Sketch)
      {
        // Remove sketch from ordered list if present and from registry/store
        m_doc->removeItem(di);
        m_doc->removeSketchById(di->id());
      }
    }
    // Deduplicate ids
    std::sort(toUnsuppressIds.begin(), toUnsuppressIds.end());
    toUnsuppressIds.erase(std::unique(toUnsuppressIds.begin(), toUnsuppressIds.end()), toUnsuppressIds.end());
    // For each recorded source id, unsuppress the feature if no remaining MoveFeature still references it
    if (!toUnsuppressIds.empty())
    {
      const auto& feats = m_doc->features();
      for (DocumentItem::Id sid : toUnsuppressIds)
      {
        Handle(Feature) src;
        for (NCollection_Sequence<Handle(Feature)>::Iterator fit(feats); fit.More(); fit.Next())
        {
          const Handle(Feature)& ff = fit.Value();
          if (!ff.IsNull() && ff->id() == sid) { src = ff; break; }
        }
        if (src.IsNull()) continue;
        // Check if any MoveFeature still references this source id
        bool stillReferenced = false;
        for (NCollection_Sequence<Handle(Feature)>::Iterator fit(feats); fit.More(); fit.Next())
        {
          Handle(MoveFeature) remMf = Handle(MoveFeature)::DownCast(fit.Value());
          if (!remMf.IsNull() && remMf->sourceId() == sid) { stillReferenced = true; break; }
        }
        if (!stillReferenced && src->isSuppressed())
        {
          src->setSuppressed(false);
        }
      }
    }
    m_doc->recompute();
    syncViewerFromDoc(true);
    refreshFeatureList();
  });
  connect(m_history, &FeatureHistoryPanel::requestSelectItem, [this](const Handle(DocumentItem)& it) {
    if (Handle(Feature) f = Handle(Feature)::DownCast(it); !f.IsNull())
      selectFeatureInViewer(f);
  });
  // Tree panel selection -> viewer select
  connect(m_treePanel, &DocumentTreePanel::requestSelectItem, [this](const Handle(DocumentItem)& it) {
    if (Handle(Feature) f = Handle(Feature)::DownCast(it); !f.IsNull())
      selectFeatureInViewer(f);
  });
  // Sync selection from viewer back to list
  connect(m_viewer, &OcctQOpenGLWidgetViewer::selectionChanged, [this]() {
    Handle(AIS_Shape) sel = m_viewer->selectedShape();
    if (sel.IsNull()) { if (m_history) m_history->selectItem(Handle(DocumentItem)()); return; }
    if (m_bodyToFeature.Contains(sel))
    {
      Handle(Feature) f = Handle(Feature)::DownCast(m_bodyToFeature.FindFromKey(sel));
      if (!f.IsNull() && m_history) m_history->selectItem(Handle(DocumentItem)(f));
    }
  });
}

TabPage::~TabPage() = default;

void TabPage::syncViewerFromDoc(bool toUpdate)
{
  if (!m_viewer) return;
  // Ensure gizmos reflect current document Datum
  if (m_doc) { m_viewer->setDatum(m_doc->datum()); }
  m_featureToBody.Clear();
  m_bodyToFeature.Clear();
  m_sketchToHandle.clear();
  m_viewer->clearBodies(false);
  m_viewer->clearSketches(false);
  for (NCollection_Sequence<Handle(Feature)>::Iterator it(m_doc->features()); it.More(); it.Next())
  {
    const Handle(Feature)& f = it.Value(); if (f.IsNull()) continue;
    if (f->isSuppressed()) continue; // do not display suppressed features
    Handle(AIS_Shape) body = m_viewer->addShape(f->shape(), AIS_Shaded, 0, false);
    m_featureToBody.Add(f, body);
    m_bodyToFeature.Add(body, f);
  }
  // Display registered sketches after features
  for (const auto& sk : m_doc->sketches())
  {
    Handle(AIS_Shape) h = m_viewer->addSketch(sk);
    if (!h.IsNull()) m_sketchToHandle[sk->id()] = h;
  }
  if (toUpdate)
  {
    m_viewer->Context()->UpdateCurrentViewer();
    m_viewer->View()->Invalidate();
    m_viewer->update();
  }
}

void TabPage::refreshFeatureList()
{
  if (m_history) m_history->refreshFromDocument();
  if (m_treePanel) m_treePanel->refreshFromDocument();
}

void TabPage::refreshDocumentTree()
{
  if (m_treePanel) m_treePanel->refreshFromDocument();
}

void TabPage::selectFeatureInViewer(const Handle(Feature)& f)
{
  if (f.IsNull() || !m_viewer) return;
  if (!m_featureToBody.Contains(f)) return;
  Handle(AIS_Shape) body = Handle(AIS_Shape)::DownCast(m_featureToBody.FindFromKey(f));
  if (body.IsNull()) return;
  auto ctx = m_viewer->Context();
  ctx->ClearSelected(false);
  ctx->AddOrRemoveSelected(body, false);
  m_viewer->Context()->UpdateCurrentViewer();
  m_viewer->View()->Invalidate();
  m_viewer->update();
}

void TabPage::activateMove()
{
  if (!m_viewer) return;
  Handle(AIS_Shape) sel = m_viewer->selectedShape();
  if (sel.IsNull()) return;
  if (!m_bodyToFeature.Contains(sel)) return;
  Handle(Feature) src = Handle(Feature)::DownCast(m_bodyToFeature.FindFromKey(sel));
  if (src.IsNull()) return;

  // Show manipulator and connect finish signal to add MoveFeature
  m_viewer->showManipulator(sel);
  // Drop any previous connection(s) for this signal to avoid duplicate commits
  QObject::disconnect(m_viewer, &OcctQOpenGLWidgetViewer::manipulatorFinished, this, nullptr);
  if (m_connManipFinished) { QObject::disconnect(m_connManipFinished); m_connManipFinished = QMetaObject::Connection(); }
  // Connect once per activation; capture Source by value
  m_connManipFinished = QObject::connect(m_viewer, &OcctQOpenGLWidgetViewer::manipulatorFinished, this, [this, src](const gp_Trsf& tr) {
    // Commit at confirm only
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
    // Store exact transform to avoid Euler reconstruction errors
    mf->setDeltaTrsf(tr);
    // Also store decomposed params for UI/serialization readability
    mf->setTranslation(t.X(), t.Y(), t.Z());
    mf->setRotation(rx, ry, rz);
    // Hide source in the scene; result should appear as a single moved body
    src->setSuppressed(true);
    m_doc->addFeature(mf);
    m_doc->recompute();
    syncViewerFromDoc(true);
    refreshFeatureList();
    // Disconnect to prevent multiple triggers on subsequent confirmations
    if (m_connManipFinished) { QObject::disconnect(m_connManipFinished); m_connManipFinished = QMetaObject::Connection(); }
  });
}

void TabPage::confirmMove()
{
  if (!m_viewer) return;
  if (!m_viewer->isManipulatorActive()) return;
  m_viewer->confirmManipulator();
  // Keep state clean in case of edge conditions
  if (m_connManipFinished) { QObject::disconnect(m_connManipFinished); m_connManipFinished = QMetaObject::Connection(); }
}

void TabPage::cancelMove()
{
  if (!m_viewer) return;
  if (!m_viewer->isManipulatorActive()) return;
  // Cancel move means no commit; drop any pending connection
  if (m_connManipFinished) { QObject::disconnect(m_connManipFinished); m_connManipFinished = QMetaObject::Connection(); }
  m_viewer->cancelManipulator();
  // Reset any temporary local transforms by resyncing from document
  syncViewerFromDoc(true);
}
