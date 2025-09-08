#include "FeatureHistoryPanel.h"

#include "TabPageWidget.h"

#include <BoxFeature.h>
#include <CylinderFeature.h>
#include <Document.h>
#include <MoveFeature.h>
#include <PlaneFeature.h>

#include <Standard_WarningsDisable.hxx>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QContextMenuEvent>
#include <QInputDialog>
#include <Standard_WarningsRestore.hxx>

#include <AIS_Shape.hxx>
#include <AppSettings.h>

FeatureHistoryPanel::FeatureHistoryPanel(TabPageWidget* page, QWidget* parent)
  : QWidget(parent), m_page(page)
{
  auto* lay = new QVBoxLayout(this);
  lay->setContentsMargins(0, 0, 0, 0);
  auto* btnRow = new QHBoxLayout();
  m_btnRemove = new QPushButton("Remove", this);
  btnRow->addStretch(1);
  btnRow->addWidget(m_btnRemove);
  lay->addLayout(btnRow);

  m_list = new QListWidget(this);
  m_list->setSelectionMode(QAbstractItemView::SingleSelection);
  m_list->installEventFilter(this); // for Delete key
  m_list->setContextMenuPolicy(Qt::CustomContextMenu);
  lay->addWidget(m_list, 1);

  connect(m_list, &QListWidget::itemSelectionChanged, this, &FeatureHistoryPanel::onSelectionChanged);
  connect(m_btnRemove, &QPushButton::clicked, this, &FeatureHistoryPanel::onRemoveClicked);
  connect(m_list, &QListWidget::customContextMenuRequested, this, &FeatureHistoryPanel::onContextMenuRequested);
}

void FeatureHistoryPanel::refreshFromDocument()
{
  m_list->clear();
  m_rowHandles.Clear();
  if (m_page == nullptr) return;
  const auto& seq = m_page->doc().items();
  for (NCollection_Sequence<Handle(DocumentItem)>::Iterator it(seq); it.More(); it.Next())
  {
    const Handle(DocumentItem)& di = it.Value();
    // Optional filter: hide Datum-related features when disabled in settings
    if (Handle(Feature) f = Handle(Feature)::DownCast(di); !f.IsNull())
    {
      if (!AppSettings::instance().showDatumRelatedItems() && f->isDatumRelated())
        continue;
    }
    m_rowHandles.Append(di);
    m_list->addItem(itemDisplayText(di));
  }
}

NCollection_Sequence<Handle(DocumentItem)> FeatureHistoryPanel::selectedItems() const
{
  NCollection_Sequence<Handle(DocumentItem)> result;
  if (!m_list) return result;
  auto items = m_list->selectedItems();
  if (items.isEmpty()) return result;
  int row = m_list->row(items.first());
  if (row >= 0 && row < m_rowHandles.Size())
  {
    result.Append(m_rowHandles.Value(row + 1)); // NCollection_Sequence is 1-based
  }
  return result;
}

void FeatureHistoryPanel::onSelectionChanged()
{
  auto sel = selectedItems();
  if (sel.IsEmpty()) return;
  emit requestSelectItem(sel.First());
}

void FeatureHistoryPanel::onRemoveClicked()
{
  emit requestRemoveSelected();
}

bool FeatureHistoryPanel::eventFilter(QObject* obj, QEvent* ev)
{
  if (obj == m_list && ev->type() == QEvent::KeyPress)
  {
    QKeyEvent* kev = static_cast<QKeyEvent*>(ev);
    if (kev->key() == Qt::Key_Delete || kev->key() == Qt::Key_Backspace)
    {
      emit requestRemoveSelected();
      return true;
    }
  }
  return QWidget::eventFilter(obj, ev);
}

QString FeatureHistoryPanel::itemDisplayText(const Handle(DocumentItem)& it) const
{
  if (it.IsNull()) return QString("<null>");
  if (Handle(Feature) f = Handle(Feature)::DownCast(it); !f.IsNull())
  {
  // Prefer explicit name if set
    if (!f->name().IsEmpty())
      return QString::fromLatin1(f->name().ToCString());
    // Otherwise, basic type-specific summary
    QString label;
    if (Handle(BoxFeature) bf = Handle(BoxFeature)::DownCast(f); !bf.IsNull())
      label = QString("Box [%1, %2, %3]").arg(bf->dx()).arg(bf->dy()).arg(bf->dz());
    else if (Handle(CylinderFeature) cf = Handle(CylinderFeature)::DownCast(f); !cf.IsNull())
      label = QString("Cylinder [R=%1, H=%2]").arg(cf->radius()).arg(cf->height());
    else if (Handle(MoveFeature) mf = Handle(MoveFeature)::DownCast(f); !mf.IsNull())
    {
      label = QString("Move [T=(%1,%2,%3), R=(%4,%5,%6) deg]")
                .arg(mf->tx()).arg(mf->ty()).arg(mf->tz())
                .arg(mf->rxDeg()).arg(mf->ryDeg()).arg(mf->rzDeg());
    }
    else if (Handle(PlaneFeature) pf = Handle(PlaneFeature)::DownCast(f); !pf.IsNull())
    {
      const gp_Pnt o = pf->origin();
      const gp_Dir n = pf->normal();
      label = QString("Plane [O=(%1,%2,%3), N=(%4,%5,%6), S=%7]")
                .arg(o.X()).arg(o.Y()).arg(o.Z())
                .arg(n.X()).arg(n.Y()).arg(n.Z())
                .arg(pf->size());
    }
    // Fallback to RTTI name
    if (label.isEmpty()) label = QString::fromLatin1(f->DynamicType()->Name());
    if (f->isSuppressed()) label += QStringLiteral(" [Suppressed]");
    return label;
  }
  // Non-feature items
  switch (it->kind())
  {
    case DocumentItem::Kind::Sketch:
      return QString("Sketch %1").arg(it->id());
    default:
      // Fallback to class name if available via RTTI; otherwise kind enum
      if (!it->DynamicType().IsNull())
        return QString::fromLatin1(it->DynamicType()->Name());
      return QString("Item %1").arg(it->id());
  }
}

void FeatureHistoryPanel::selectItem(const Handle(DocumentItem)& it)
{
  if (it.IsNull()) { m_list->clearSelection(); return; }
  // Find row by handle equality
  for (int i = 1; i <= m_rowHandles.Size(); ++i)
  {
    if (m_rowHandles.Value(i) == it)
    {
      m_list->setCurrentRow(i - 1, QItemSelectionModel::ClearAndSelect);
      return;
    }
  }
}

void FeatureHistoryPanel::onContextMenuRequested(const QPoint& pos)
{
  auto items = m_list->selectedItems();
  QMenu menu(this);
  QAction* actRename = menu.addAction("Rename...");
  QAction* actToggle = nullptr;
  bool haveSel = !items.isEmpty();
  if (haveSel)
  {
    int row = m_list->row(items.first());
    if (row >= 0 && row < m_rowHandles.Size())
    {
      Handle(DocumentItem) di = m_rowHandles.Value(row + 1);
      if (Handle(Feature) f = Handle(Feature)::DownCast(di); !f.IsNull())
        actToggle = menu.addAction(f->isSuppressed() ? "Unsuppress" : "Suppress");
    }
  }
  QAction* actRemove = menu.addAction("Remove");
  QAction* chosen = menu.exec(m_list->viewport()->mapToGlobal(pos));
  if (!chosen) return;
  if (chosen == actRename) { doRenameSelected(); }
  else if (chosen == actRemove) { onRemoveClicked(); }
  else if (chosen == actToggle) { doToggleSuppressSelected(); }
}

void FeatureHistoryPanel::doRenameSelected()
{
  auto items = m_list->selectedItems();
  if (items.isEmpty() || m_page == nullptr) return;
  int row = m_list->row(items.first());
  if (row < 0 || row >= m_rowHandles.Size()) return;
  Handle(DocumentItem) di = m_rowHandles.Value(row + 1);
  Handle(Feature) f = Handle(Feature)::DownCast(di);
  if (f.IsNull()) return;
  bool ok = false;
  QString current = itemDisplayText(f);
  // Use only name component if present
  if (!f->name().IsEmpty()) current = QString::fromLatin1(f->name().ToCString());
  QString text = QInputDialog::getText(this, "Rename Feature", "Name:", QLineEdit::Normal, current, &ok);
  if (!ok) return;
  // Trim and set
  text = text.trimmed();
  if (text.isEmpty()) return;
  f->setName(TCollection_AsciiString(text.toLatin1().constData()));
  refreshFromDocument();
}

void FeatureHistoryPanel::doToggleSuppressSelected()
{
  auto items = m_list->selectedItems();
  if (items.isEmpty() || m_page == nullptr) return;
  int row = m_list->row(items.first());
  if (row < 0 || row >= m_rowHandles.Size()) return;
  Handle(DocumentItem) di = m_rowHandles.Value(row + 1);
  Handle(Feature) f = Handle(Feature)::DownCast(di);
  if (f.IsNull()) return;
  f->setSuppressed(!f->isSuppressed());
  // Recompute document and resync viewer + list
  m_page->doc().recompute();
  m_page->syncViewerFromDoc(true);
  refreshFromDocument();
}
