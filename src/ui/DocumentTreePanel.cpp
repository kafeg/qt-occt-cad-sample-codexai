#include "DocumentTreePanel.h"

#include "TabPage.h"
#include <OcctQOpenGLWidgetViewer.h>

#include <BoxFeature.h>
#include <CylinderFeature.h>
#include <MoveFeature.h>
#include <PlaneFeature.h>
#include <PointFeature.h>
#include <AxeFeature.h>
#include <Sketch.h>
#include <Document.h>
#include <Datum.h>
#include <AppSettings.h>

#include <Standard_WarningsDisable.hxx>
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QHeaderView>
#include <QStringList>
#include <QSignalBlocker>
#include <Standard_WarningsRestore.hxx>

DocumentTreePanel::DocumentTreePanel(TabPage* page, QWidget* parent)
  : QWidget(parent), m_page(page)
{
  auto* lay = new QVBoxLayout(this);
  lay->setContentsMargins(0, 0, 0, 0);
  m_tree = new QTreeWidget(this);
  m_tree->setHeaderHidden(true);
  m_tree->setSelectionMode(QAbstractItemView::SingleSelection);
  lay->addWidget(m_tree);

  connect(m_tree, &QTreeWidget::itemSelectionChanged, this, &DocumentTreePanel::onSelectionChanged);
  connect(m_tree, &QTreeWidget::itemChanged, this, &DocumentTreePanel::onItemChanged);
}

void DocumentTreePanel::refreshFromDocument()
{
  if (!m_tree) return;
  QSignalBlocker blocker(m_tree);
  m_tree->clear();
  if (m_page == nullptr) return;

  QTreeWidgetItem* root = new QTreeWidgetItem(QStringList() << QStringLiteral("Document"));
  m_tree->addTopLevelItem(root);

  // Origin section (datum controls)
  QTreeWidgetItem* datum = new QTreeWidgetItem(root, QStringList() << QStringLiteral("Origin"));
  if (auto d = m_page->doc().datum())
  {
    // Checkable visibility toggles
    auto makeToggle = [&](const QString& label, bool checked, int tag) {
      QTreeWidgetItem* it = new QTreeWidgetItem(datum, QStringList() << label);
      it->setFlags(it->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
      it->setCheckState(0, checked ? Qt::Checked : Qt::Unchecked);
      it->setData(0, Qt::UserRole + 1, tag);
      return it;
    };
    enum ToggleTag { TagTrihedron = 1, TagOrigin = 2,
                     TagPlaneXY = 3, TagPlaneXZ = 4, TagPlaneYZ = 5,
                     TagTriX = 6, TagTriY = 7, TagTriZ = 8 };
    // Helper to attach feature id by name for selection behavior
    auto setFeatureIdByName = [&](QTreeWidgetItem* node, const char* targetName) {
      auto& doc = m_page->doc();
      const auto& items = doc.items();
      for (NCollection_Sequence<Handle(DocumentItem)>::Iterator it(items); it.More(); it.Next())
      {
        Handle(Feature) f = Handle(Feature)::DownCast(it.Value());
        if (f.IsNull()) continue;
        if (f->name().IsEmpty()) continue;
        const QString fname = QString::fromLatin1(f->name().ToCString());
        if (fname == QString::fromLatin1(targetName))
        {
          node->setData(0, Qt::UserRole, QVariant::fromValue<qulonglong>(static_cast<qulonglong>(f->id())));
          break;
        }
      }
    };
    // Removed overall trihedron toggle from UI; keep per-axis only.
    // Order: point, axes, planes
    QTreeWidgetItem* itOrigin = makeToggle(QStringLiteral("Origin Point"), d->showOriginPoint(), TagOrigin);
    setFeatureIdByName(itOrigin, "Origin");
    QTreeWidgetItem* itAxX = makeToggle(QStringLiteral("  X axis"), d->showTrihedronAxisX(), TagTriX);
    setFeatureIdByName(itAxX, "Axis X");
    QTreeWidgetItem* itAxY = makeToggle(QStringLiteral("  Y axis"), d->showTrihedronAxisY(), TagTriY);
    setFeatureIdByName(itAxY, "Axis Y");
    QTreeWidgetItem* itAxZ = makeToggle(QStringLiteral("  Z axis"), d->showTrihedronAxisZ(), TagTriZ);
    setFeatureIdByName(itAxZ, "Axis Z");
    QTreeWidgetItem* itPlXY = makeToggle(QStringLiteral("  Plane XY"), d->showPlaneXY(), TagPlaneXY);
    setFeatureIdByName(itPlXY, "Plane XY");
    QTreeWidgetItem* itPlXZ = makeToggle(QStringLiteral("  Plane XZ"), d->showPlaneXZ(), TagPlaneXZ);
    setFeatureIdByName(itPlXZ, "Plane XZ");
    QTreeWidgetItem* itPlYZ = makeToggle(QStringLiteral("  Plane YZ"), d->showPlaneYZ(), TagPlaneYZ);
    setFeatureIdByName(itPlYZ, "Plane YZ");
  }

  // Bodies (features)
  QTreeWidgetItem* bodies = new QTreeWidgetItem(root, QStringList() << QStringLiteral("Bodies"));
  const auto& seq = m_page->doc().items();
  for (NCollection_Sequence<Handle(DocumentItem)>::Iterator it(seq); it.More(); it.Next())
  {
    const Handle(DocumentItem)& di = it.Value();
    Handle(Feature) f = Handle(Feature)::DownCast(di);
    if (f.IsNull()) continue;
    // Optional filter: hide Datum-related features when disabled in settings
    if (!AppSettings::instance().showDatumRelatedItems() && f->isDatumRelated())
      continue;
    auto* node = new QTreeWidgetItem(bodies, QStringList() << itemDisplayText(f));
    // Store document id to resolve selection later
    node->setData(0, Qt::UserRole, QVariant::fromValue<qulonglong>(static_cast<qulonglong>(f->id())));
  }

  // Sketches
  QTreeWidgetItem* sketchesNode = new QTreeWidgetItem(root, QStringList() << QStringLiteral("Sketches"));
  const auto sketches = m_page->doc().sketches();
  for (const auto& sk : sketches)
  {
    const auto sid = sk ? sk->id() : 0;
    (void)new QTreeWidgetItem(sketchesNode, QStringList() << QString("Sketch %1").arg(static_cast<qulonglong>(sid)));
  }

  root->setExpanded(true);
  datum->setExpanded(true);
  bodies->setExpanded(true);
  sketchesNode->setExpanded(true);
}

void DocumentTreePanel::onSelectionChanged()
{
  if (!m_tree || !m_page) return;
  auto items = m_tree->selectedItems();
  if (items.isEmpty()) return;
  QTreeWidgetItem* it = items.first();
  QVariant v = it->data(0, Qt::UserRole);
  if (!v.isValid()) return;
  const qulonglong id = v.value<qulonglong>();
  if (id == 0) return;
  // Find matching Handle(Feature) by id in the document items
  const auto& seq = m_page->doc().items();
  for (NCollection_Sequence<Handle(DocumentItem)>::Iterator dit(seq); dit.More(); dit.Next())
  {
    const Handle(DocumentItem)& di = dit.Value();
    Handle(Feature) f = Handle(Feature)::DownCast(di);
    if (!f.IsNull() && static_cast<qulonglong>(f->id()) == id)
    {
      emit requestSelectItem(Handle(DocumentItem)(f));
      return;
    }
  }
}

void DocumentTreePanel::onItemChanged(QTreeWidgetItem* item, int column)
{
  if (!item || !m_page || !m_page->viewer()) return;
  // Only handle our checkable toggles (identified by UserRole+1)
  QVariant tagVar = item->data(0, Qt::UserRole + 1);
  if (!tagVar.isValid()) return;
  auto d = m_page->doc().datum();
  if (!d) return;
  const bool on = (item->checkState(0) == Qt::Checked);
  const int tag = tagVar.toInt();
  switch (tag)
  {
    case 1: d->setShowTrihedronAxes(on); break;
    case 2:
    {
      d->setShowOriginPoint(on);
      // Mirror origin visibility only if the PointFeature exists; do NOT recreate if deleted.
      auto& doc = m_page->doc();
      Handle(PointFeature) pf;
      for (NCollection_Sequence<Handle(DocumentItem)>::Iterator it(doc.items()); it.More(); it.Next())
      {
        Handle(PointFeature) tmp = Handle(PointFeature)::DownCast(it.Value());
        if (!tmp.IsNull()) { pf = tmp; break; }
      }
      bool changed = false;
      if (on)
      {
        if (!pf.IsNull()) { pf->setSuppressed(false); changed = true; }
      }
      else
      {
        if (!pf.IsNull()) { pf->setSuppressed(true); changed = true; }
      }
      if (changed) { doc.recompute(); m_page->syncViewerFromDoc(true); }
      break;
    }
    case 3: // Plane XY
    case 4: // Plane XZ
    case 5: // Plane YZ
    {
      // Update Datum flags
      if (tag == 3) d->setShowPlaneXY(on);
      if (tag == 4) d->setShowPlaneXZ(on);
      if (tag == 5) d->setShowPlaneYZ(on);
      // Suppress/unsuppress matching plane features by name
      auto& doc = m_page->doc();
      const char* targetName = (tag == 3 ? "Plane XY" : (tag == 4 ? "Plane XZ" : "Plane YZ"));
      for (NCollection_Sequence<Handle(DocumentItem)>::Iterator it(doc.items()); it.More(); it.Next())
      {
        Handle(PlaneFeature) pf = Handle(PlaneFeature)::DownCast(it.Value());
        if (pf.IsNull()) continue;
        if (!pf->name().IsEmpty() && TCollection_AsciiString(targetName).IsEqual(pf->name()))
        {
          pf->setSuppressed(!on);
          break;
        }
      }
      doc.recompute();
      m_page->syncViewerFromDoc(true);
      break;
    }
    case 6: // X axis
    case 7: // Y axis
    case 8: // Z axis
    {
      // Update Datum flags
      if (tag == 6) d->setShowTrihedronAxisX(on);
      if (tag == 7) d->setShowTrihedronAxisY(on);
      if (tag == 8) d->setShowTrihedronAxisZ(on);
      // Suppress/unsuppress matching AxeFeature by name
      auto& doc = m_page->doc();
      const char* targetName = (tag == 6 ? "Axis X" : (tag == 7 ? "Axis Y" : "Axis Z"));
      for (NCollection_Sequence<Handle(DocumentItem)>::Iterator it(doc.items()); it.More(); it.Next())
      {
        Handle(AxeFeature) ax = Handle(AxeFeature)::DownCast(it.Value());
        if (ax.IsNull()) continue;
        if (!ax->name().IsEmpty() && TCollection_AsciiString(targetName).IsEqual(ax->name()))
        {
          ax->setSuppressed(!on);
          break;
        }
      }
      doc.recompute();
      m_page->syncViewerFromDoc(true);
      break;
    }
    default: return;
  }
  // Reinstall datum gizmos in viewer to reflect toggles
  m_page->viewer()->setDatum(d);
}

void DocumentTreePanel::selectItem(const Handle(DocumentItem)& it)
{
  if (!m_tree) return;
  if (it.IsNull()) { m_tree->clearSelection(); return; }
  const qulonglong targetId = static_cast<qulonglong>(it->id());
  // Depth-first search for item with matching id stored in UserRole
  std::function<QTreeWidgetItem*(QTreeWidgetItem*)> findInSubtree;
  findInSubtree = [&](QTreeWidgetItem* node) -> QTreeWidgetItem*
  {
    if (!node) return nullptr;
    QVariant v = node->data(0, Qt::UserRole);
    if (v.isValid() && v.value<qulonglong>() == targetId) return node;
    const int n = node->childCount();
    for (int i = 0; i < n; ++i)
    {
      if (QTreeWidgetItem* found = findInSubtree(node->child(i))) return found;
    }
    return nullptr;
  };
  QTreeWidgetItem* found = nullptr;
  for (int i = 0; i < m_tree->topLevelItemCount() && !found; ++i)
  {
    found = findInSubtree(m_tree->topLevelItem(i));
  }
  if (found)
  {
    m_tree->setCurrentItem(found, 0, QItemSelectionModel::ClearAndSelect);
  }
}

QString DocumentTreePanel::itemDisplayText(const Handle(DocumentItem)& it) const
{
  if (it.IsNull()) return QString("<null>");
  if (Handle(Feature) f = Handle(Feature)::DownCast(it); !f.IsNull())
  {
    if (!f->name().IsEmpty())
      return QString::fromLatin1(f->name().ToCString());
    QString label;
    if (Handle(BoxFeature) bf = Handle(BoxFeature)::DownCast(f); !bf.IsNull())
      label = QString("Box [%1, %2, %3]").arg(bf->dx()).arg(bf->dy()).arg(bf->dz());
    else if (Handle(CylinderFeature) cf = Handle(CylinderFeature)::DownCast(f); !cf.IsNull())
      label = QString("Cylinder [R=%1, H=%2]").arg(cf->radius()).arg(cf->height());
    else if (Handle(MoveFeature) mf = Handle(MoveFeature)::DownCast(f); !mf.IsNull())
      label = QString("Move [T=(%1,%2,%3), R=(%4,%5,%6) deg]")
                .arg(mf->tx()).arg(mf->ty()).arg(mf->tz())
                .arg(mf->rxDeg()).arg(mf->ryDeg()).arg(mf->rzDeg());
    else if (Handle(PlaneFeature) pf = Handle(PlaneFeature)::DownCast(f); !pf.IsNull())
    {
      const gp_Pnt o = pf->origin();
      const gp_Dir n = pf->normal();
      label = QString("Plane [O=(%1,%2,%3), N=(%4,%5,%6), S=%7]")
                .arg(o.X()).arg(o.Y()).arg(o.Z())
                .arg(n.X()).arg(n.Y()).arg(n.Z())
                .arg(pf->size());
    }
    else if (Handle(PointFeature) pt = Handle(PointFeature)::DownCast(f); !pt.IsNull())
    {
      const gp_Pnt o = pt->origin();
      label = QString("Point [O=(%1,%2,%3), R=%4]")
                .arg(o.X()).arg(o.Y()).arg(o.Z())
                .arg(pt->radius());
    }
    if (label.isEmpty()) label = QString::fromLatin1(f->DynamicType()->Name());
    if (f->isSuppressed()) label += QStringLiteral(" [Suppressed]");
    return label;
  }
  if (!it->DynamicType().IsNull())
    return QString::fromLatin1(it->DynamicType()->Name());
  return QString("Item %1").arg(it->id());
}
