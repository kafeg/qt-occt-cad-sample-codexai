// Per-tab container: owns a Document and embeds a Viewer
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

// Per-tab page: owns a Document and embeds a reusable 3D viewer
class TabPage : public QWidget
{
  Q_OBJECT
public:
  explicit TabPage(QWidget* parent = nullptr);
  ~TabPage() override;

  OcctQOpenGLWidgetViewer* viewer() const { return m_viewer; } // access embedded viewer
  Document&                doc()          { return *m_doc; }   // access document

  TColStd_IndexedDataMapOfTransientTransient& featureToBody() { return m_featureToBody; } // Feature -> AIS map
  TColStd_IndexedDataMapOfTransientTransient& bodyToFeature() { return m_bodyToFeature; } // AIS -> Feature map

  // Sync viewer bodies from the Document (rebuild AIS shapes). Optionally update immediately.
  void syncViewerFromDoc(bool toUpdate = true);

  // Rebuild the feature history panel list from Document
  void refreshFeatureList();

  // Select a feature's AIS body in the viewer
  void selectFeatureInViewer(const Handle(Feature)& f);

  // Activate interactive move on current selection; adds MoveFeature on finish
  void activateMove();
  void confirmMove();
  void cancelMove();

private:
  OcctQOpenGLWidgetViewer*                 m_viewer = nullptr; // OCCT viewer widget
  std::unique_ptr<Document>                m_doc;              // model document
  TColStd_IndexedDataMapOfTransientTransient m_featureToBody;  // feature -> body
  TColStd_IndexedDataMapOfTransientTransient m_bodyToFeature;  // body -> feature
  std::unordered_map<DocumentItem::Id, Handle(AIS_Shape)> m_sketchToHandle; // sketch id -> AIS handle
  FeatureHistoryPanel*                     m_history = nullptr;// feature history panel
  QMetaObject::Connection                  m_connManipFinished; // manipulatorFinished connection
};
