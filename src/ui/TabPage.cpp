#include "TabPage.h"

#include <OcctQOpenGLWidgetViewer.h>
#include "FeatureHistoryPanel.h"

#include <Standard_WarningsDisable.hxx>
#include <QVBoxLayout>
#include <QSplitter>
#include <Standard_WarningsRestore.hxx>

#include <Document.h>
#include <Sketch.h>
#include <AIS_Shape.hxx>

TabPage::TabPage(QWidget* parent)
  : QWidget(parent)
{
  auto* lay = new QVBoxLayout(this);
  lay->setContentsMargins(0, 0, 0, 0);
  auto* split = new QSplitter(Qt::Horizontal, this);
  split->setChildrenCollapsible(false);
  m_history = new FeatureHistoryPanel(this, split);
  m_viewer = new OcctQOpenGLWidgetViewer(split);
  // Position camera towards origin for a sensible default view
  // Start closer to the origin (e.g., 20x closer than default)
  m_viewer->resetViewToOrigin(0.05);
  split->addWidget(m_history);
  split->addWidget(m_viewer);
  split->setStretchFactor(0, 0);
  split->setStretchFactor(1, 1);
  split->setSizes({200, 800});
  lay->addWidget(split);
  m_doc = std::make_unique<Document>();

  // Connect panel actions
  connect(m_history, &FeatureHistoryPanel::requestRemoveSelected, [this]() {
    // Remove selected items; currently only Feature removal is supported
    auto sel = m_history->selectedItems();
    for (NCollection_Sequence<Handle(DocumentItem)>::Iterator it(sel); it.More(); it.Next())
    {
      const Handle(DocumentItem)& di = it.Value();
      if (Handle(Feature) f = Handle(Feature)::DownCast(di); !f.IsNull())
        m_doc->removeFeature(f);
    }
    m_doc->recompute();
    syncViewerFromDoc(true);
    refreshFeatureList();
  });
  connect(m_history, &FeatureHistoryPanel::requestSelectItem, [this](const Handle(DocumentItem)& it) {
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
