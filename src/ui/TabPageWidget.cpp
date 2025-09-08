#include "TabPageWidget.h"

#include <OcctQOpenGLWidgetViewer.h>
#include "FeatureHistoryPanel.h"
#include "DocumentTreePanel.h"
#include "StatsPanel.h"
#include "TabPageModel.h"

#include <Standard_WarningsDisable.hxx>
#include <QVBoxLayout>
#include <QSplitter>
#include <Standard_WarningsRestore.hxx>

#include <Document.h>
#include <Sketch.h>
#include <AIS_Shape.hxx>
#include <MoveFeature.h>
#include <PlaneFeature.h>
#include <PointFeature.h>
#include <AxeFeature.h>
#include <Datum.h>
#include <Quantity_Color.hxx>
#include <AppSettings.h>

#include <algorithm>
#include <vector>

TabPageWidget::TabPageWidget(QWidget* parent)
  : QWidget(parent)
{
  auto* lay = new QVBoxLayout(this);
  lay->setContentsMargins(0, 0, 0, 0);
  auto* split = new QSplitter(Qt::Horizontal, this);
  split->setChildrenCollapsible(false);

  // Left sidebar: vertical splitter with Document tree over Feature history
  QSplitter* left = new QSplitter(Qt::Vertical, split);
  left->setChildrenCollapsible(false);
  m_treePanel = new DocumentTreePanel(this, left);
  m_history   = new FeatureHistoryPanel(this, left);
  left->addWidget(m_treePanel);
  left->addWidget(m_history);
  left->setStretchFactor(0, 1);
  left->setStretchFactor(1, 1);

  // Create model and attach viewer (no widget classes in model header)
  m_model = new TabPageModel(this);
  m_viewer = qobject_cast<OcctQOpenGLWidgetViewer*>(m_model->createViewer(split));
  m_stats  = new StatsPanel(split);
  split->addWidget(left);
  split->addWidget(m_viewer);
  split->addWidget(m_stats);
  split->setStretchFactor(0, 0);
  split->setStretchFactor(1, 1);
  split->setStretchFactor(2, 0);
  split->setSizes({260, 800, 200});
  lay->addWidget(split);

  // Attach stats to viewer
  if (m_stats && m_viewer) { m_stats->attachViewer(m_viewer); }
  connectModelSignals();
  // Populate viewer initially so datum (origin planes/axes/point) are visible by default
  m_model->syncViewerFromDoc(true);

  // Connect panel actions for removal/selection
  connect(m_history, &FeatureHistoryPanel::requestRemoveSelected, [this]() {
    // Remove selected items; currently supports Feature and Sketch removal
    auto sel = m_history->selectedItems();
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
        doc().removeFeature(f);
      }
      else if (!di.IsNull() && di->kind() == DocumentItem::Kind::Sketch)
      {
        doc().removeItem(di);
        doc().removeSketchById(di->id());
      }
    }
    // Deduplicate candidates
    std::sort(toUnsuppressIds.begin(), toUnsuppressIds.end());
    toUnsuppressIds.erase(std::unique(toUnsuppressIds.begin(), toUnsuppressIds.end()), toUnsuppressIds.end());
    if (!toUnsuppressIds.empty())
    {
      const auto& feats = doc().features();
      for (DocumentItem::Id sid : toUnsuppressIds)
      {
        Handle(Feature) src;
        for (NCollection_Sequence<Handle(Feature)>::Iterator fit(feats); fit.More(); fit.Next())
        {
          const Handle(Feature)& ff = fit.Value();
          if (!ff.IsNull() && ff->id() == sid) { src = ff; break; }
        }
        if (src.IsNull()) continue;
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
    doc().recompute();
    syncViewerFromDoc(true);
    refreshFeatureList();
  });
  connect(m_history, &FeatureHistoryPanel::requestSelectItem, [this](const Handle(DocumentItem)& it) {
    if (Handle(Feature) f = Handle(Feature)::DownCast(it); !f.IsNull())
    {
      selectFeatureInViewer(f);
    }
  });
}

TabPageWidget::~TabPageWidget() = default;

Document& TabPageWidget::doc()
{
  return m_model->doc();
}

void TabPageWidget::connectModelSignals()
{
  connect(m_model, &TabPageModel::featureListChanged, this, [this]() { refreshFeatureList(); });
  connect(m_model, &TabPageModel::documentTreeChanged, this, [this]() { refreshDocumentTree(); });
}

void TabPageWidget::syncViewerFromDoc(bool toUpdate)
{
  m_model->syncViewerFromDoc(toUpdate);
}

void TabPageWidget::refreshFeatureList()
{
  if (m_history) m_history->refreshFromDocument();
  if (m_treePanel) m_treePanel->refreshFromDocument();
}

void TabPageWidget::refreshDocumentTree()
{
  if (m_treePanel) m_treePanel->refreshFromDocument();
}

void TabPageWidget::selectFeatureInViewer(const Handle(Feature)& f)
{
  m_model->selectFeatureInViewer(f);
}

void TabPageWidget::activateMove()
{
  m_model->activateMove();
}

void TabPageWidget::confirmMove()
{
  m_model->confirmMove();
}

void TabPageWidget::cancelMove()
{
  m_model->cancelMove();
}
