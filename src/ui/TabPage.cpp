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
  split->addWidget(m_history);
  split->addWidget(m_viewer);
  split->setStretchFactor(0, 0);
  split->setStretchFactor(1, 1);
  split->setSizes({200, 800});
  lay->addWidget(split);
  m_doc = std::make_unique<Document>();

  // Connect panel actions
  connect(m_history, &FeatureHistoryPanel::requestRemoveSelected, [this]() {
    // Remove selected features from document, recompute, and resync viewer + panel
    auto sel = m_history->selectedFeatures();
    for (NCollection_Sequence<Handle(Feature)>::Iterator it(sel); it.More(); it.Next())
    {
      const Handle(Feature)& f = it.Value();
      if (!f.IsNull())
        m_doc->removeFeature(f);
    }
    m_doc->recompute();
    syncViewerFromDoc(true);
    refreshFeatureList();
  });
  connect(m_history, &FeatureHistoryPanel::requestSelectFeature, [this](const Handle(Feature)& f) {
    selectFeatureInViewer(f);
  });
  // Sync selection from viewer back to list
  connect(m_viewer, &OcctQOpenGLWidgetViewer::selectionChanged, [this]() {
    Handle(AIS_Shape) sel = m_viewer->selectedShape();
    if (sel.IsNull()) { if (m_history) m_history->selectFeature(Handle(Feature)()); return; }
    if (m_bodyToFeature.Contains(sel))
    {
      Handle(Feature) f = Handle(Feature)::DownCast(m_bodyToFeature.FindFromKey(sel));
      if (!f.IsNull() && m_history) m_history->selectFeature(f);
    }
  });
}

TabPage::~TabPage() = default;

void TabPage::syncViewerFromDoc(bool toUpdate)
{
  if (!m_viewer) return;
  m_featureToBody.Clear();
  m_bodyToFeature.Clear();
  m_viewer->clearBodies(false);
  for (NCollection_Sequence<Handle(Feature)>::Iterator it(m_doc->features()); it.More(); it.Next())
  {
    const Handle(Feature)& f = it.Value(); if (f.IsNull()) continue;
    if (f->isSuppressed()) continue; // do not display suppressed features
    Handle(AIS_Shape) body = m_viewer->addShape(f->shape(), AIS_Shaded, 0, false);
    m_featureToBody.Add(f, body);
    m_bodyToFeature.Add(body, f);
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
