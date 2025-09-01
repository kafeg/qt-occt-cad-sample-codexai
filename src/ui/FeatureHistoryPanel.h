// Simple feature-history panel: lists Document features and supports selection/removal
#pragma once

#include <Standard_WarningsDisable.hxx>
#include <QWidget>
#include <Standard_WarningsRestore.hxx>

#include <NCollection_Sequence.hxx>
#include <Feature.h>

class QListWidget;
class QPushButton;
class TabPage;

class FeatureHistoryPanel : public QWidget
{
  Q_OBJECT
public:
  explicit FeatureHistoryPanel(TabPage* page, QWidget* parent = nullptr);

  // Rebuild the list from the current Document state
  void refreshFromDocument();

  // Return handles corresponding to current selection (may be empty)
  NCollection_Sequence<Handle(Feature)> selectedFeatures() const;

  // Select a specific feature in the list
  void selectFeature(const Handle(Feature)& f);

signals:
  void requestRemoveSelected();
  void requestSelectFeature(const Handle(Feature)& f);

private slots:
  void onSelectionChanged();
  void onRemoveClicked();
  void onContextMenuRequested(const QPoint& pos);
  void doRenameSelected();
  void doToggleSuppressSelected();

protected:
  bool eventFilter(QObject* obj, QEvent* ev) override;

private:
  QString featureDisplayText(const Handle(Feature)& f) const;

private:
  TabPage*     m_page = nullptr;
  QListWidget* m_list = nullptr;
  QPushButton* m_btnRemove = nullptr;
  // Row -> Feature handle mapping for current list state
  NCollection_Sequence<Handle(Feature)> m_rowHandles;
};
