// Per-tab container widget: composes panels and viewer, delegates logic to TabPageModel
#pragma once

#include <Standard_WarningsDisable.hxx>
#include <QWidget>
#include <QObject>
#include <Standard_WarningsRestore.hxx>

#include <TColStd_IndexedDataMapOfTransientTransient.hxx>
#include <Feature.h>
#include <AIS_Shape.hxx>
#include <memory>
#include <unordered_map>
#include <DocumentItem.h>

class Document;
class OcctQOpenGLWidgetViewer;
class FeatureHistoryPanel;
class DocumentTreePanel;
class StatsPanel;
class TabPageModel;

// Per-tab page widget: owns UI (panels + viewer) and embeds TabPageModel
class TabPageWidget : public QWidget
{
  Q_OBJECT
public:
  explicit TabPageWidget(QWidget* parent = nullptr);
  ~TabPageWidget() override;

  OcctQOpenGLWidgetViewer* viewer() const { return m_viewer; }
  Document&                doc();

  // Sync and UI refresh helpers
  void syncViewerFromDoc(bool toUpdate = true);
  void refreshFeatureList();
  void refreshDocumentTree();

  // Selection and move control
  void selectFeatureInViewer(const Handle(Feature)& f);
  void activateMove();
  void confirmMove();
  void cancelMove();

private:
  void connectModelSignals();

private:
  OcctQOpenGLWidgetViewer* m_viewer = nullptr;
  StatsPanel*               m_stats = nullptr;
  FeatureHistoryPanel*      m_history = nullptr;
  DocumentTreePanel*        m_treePanel = nullptr;
  TabPageModel*             m_model = nullptr; // owned by this widget via QObject parent
};

