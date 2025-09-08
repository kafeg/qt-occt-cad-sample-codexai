// Document tree panel: shows Datum, Bodies, Sketches in a tree
#pragma once

#include <Standard_WarningsDisable.hxx>
#include <QWidget>
#include <Standard_WarningsRestore.hxx>

#include <NCollection_Sequence.hxx>
#include <DocumentItem.h>

class QTreeWidget;
class QTreeWidgetItem;
class TabPageWidget;

class DocumentTreePanel : public QWidget
{
  Q_OBJECT
public:
  explicit DocumentTreePanel(TabPageWidget* page, QWidget* parent = nullptr);

  // Rebuild the tree from the current Document state
  void refreshFromDocument();

  // Select a specific item in the tree by handle (matches by id)
  void selectItem(const Handle(DocumentItem)& it);

signals:
  void requestSelectItem(const Handle(DocumentItem)& it);

private slots:
  void onSelectionChanged();
  void onItemChanged(QTreeWidgetItem* item, int column);

private:
  // Build label for a given item (features mainly)
  QString itemDisplayText(const Handle(DocumentItem)& it) const;

private:
  TabPageWidget* m_page = nullptr;
  QTreeWidget* m_tree = nullptr;
};
