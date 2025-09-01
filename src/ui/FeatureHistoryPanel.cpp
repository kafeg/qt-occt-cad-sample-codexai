#include "FeatureHistoryPanel.h"

#include "TabPage.h"

#include <BoxFeature.h>
#include <CylinderFeature.h>
#include <Document.h>

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

FeatureHistoryPanel::FeatureHistoryPanel(TabPage* page, QWidget* parent)
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
  const auto& seq = m_page->doc().features();
  int row = 0;
  for (NCollection_Sequence<Handle(Feature)>::Iterator it(seq); it.More(); it.Next())
  {
    const Handle(Feature)& f = it.Value();
    m_rowHandles.Append(f);
    m_list->addItem(featureDisplayText(f));
    ++row;
  }
}

NCollection_Sequence<Handle(Feature)> FeatureHistoryPanel::selectedFeatures() const
{
  NCollection_Sequence<Handle(Feature)> result;
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
  auto sel = selectedFeatures();
  if (sel.IsEmpty()) return;
  emit requestSelectFeature(sel.First());
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

QString FeatureHistoryPanel::featureDisplayText(const Handle(Feature)& f) const
{
  if (f.IsNull()) return QString("<null>");
  // Prefer explicit name if set
  if (!f->name().IsEmpty())
  {
    return QString::fromLatin1(f->name().ToCString());
  }
  // Otherwise, basic type-specific summary
  QString label;
  if (Handle(BoxFeature) bf = Handle(BoxFeature)::DownCast(f); !bf.IsNull())
  {
    label = QString("Box [%1, %2, %3]").arg(bf->dx()).arg(bf->dy()).arg(bf->dz());
  }
  else if (Handle(CylinderFeature) cf = Handle(CylinderFeature)::DownCast(f); !cf.IsNull())
  {
    label = QString("Cylinder [R=%1, H=%2]").arg(cf->radius()).arg(cf->height());
  }
  // Fallback to RTTI name
  if (label.isEmpty()) label = QString::fromLatin1(f->DynamicType()->Name());
  if (f->isSuppressed()) label += QStringLiteral(" [Suppressed]");
  return label;
}

void FeatureHistoryPanel::selectFeature(const Handle(Feature)& f)
{
  if (f.IsNull()) { m_list->clearSelection(); return; }
  // Find row by handle equality
  for (int i = 1; i <= m_rowHandles.Size(); ++i)
  {
    if (m_rowHandles.Value(i) == f)
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
      Handle(Feature) f = m_rowHandles.Value(row + 1);
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
  Handle(Feature) f = m_rowHandles.Value(row + 1);
  bool ok = false;
  QString current = featureDisplayText(f);
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
  Handle(Feature) f = m_rowHandles.Value(row + 1);
  f->setSuppressed(!f->isSuppressed());
  // Recompute document and resync viewer + list
  m_page->doc().recompute();
  m_page->syncViewerFromDoc(true);
  refreshFromDocument();
}
