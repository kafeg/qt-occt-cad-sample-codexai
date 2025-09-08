// Simple feature-history panel: lists Document features and supports selection/removal
#pragma once

#include <Standard_WarningsDisable.hxx>
#include <QWidget>
#include <Standard_WarningsRestore.hxx>

#include <NCollection_Sequence.hxx>
#include <Feature.h>
#include <DocumentItem.h>

class QListWidget;
class QPushButton;
class TabPageWidget;

class FeatureHistoryPanel : public QWidget
{
  Q_OBJECT
public:
  explicit FeatureHistoryPanel(TabPageWidget* page, QWidget* parent = nullptr);

  // Rebuild the list from the current Document state
  void refreshFromDocument();

  // Return handles corresponding to current selection (may be empty)
  NCollection_Sequence<Handle(DocumentItem)> selectedItems() const;

  // Select a specific item in the list
  void selectItem(const Handle(DocumentItem)& it);

signals:
  void requestRemoveSelected();
  void requestSelectItem(const Handle(DocumentItem)& it);

private slots:
  void onSelectionChanged();
  void onRemoveClicked();
  void onContextMenuRequested(const QPoint& pos);
  void doRenameSelected();
  void doToggleSuppressSelected();

protected:
  bool eventFilter(QObject* obj, QEvent* ev) override;

private:
  QString itemDisplayText(const Handle(DocumentItem)& it) const;

private:
  TabPageWidget* m_page = nullptr;
  QListWidget* m_list = nullptr;
  QPushButton* m_btnRemove = nullptr;
  // Row -> Item handle mapping for current list state
  NCollection_Sequence<Handle(DocumentItem)> m_rowHandles;
};
