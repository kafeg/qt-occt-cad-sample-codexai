// TabPageModel: QObject-based model/controller for a single document tab
#pragma once

#include <Standard_WarningsDisable.hxx>
#include <QObject>
#include <Standard_WarningsRestore.hxx>

#include <TColStd_IndexedDataMapOfTransientTransient.hxx>
#include <AIS_Shape.hxx>
#include <Feature.h>
#include <DocumentItem.h>
#include <memory>
#include <unordered_map>

class Document;
class Feature;

// Note: The public API avoids direct references to QWidget-derived classes
// to keep this model reusable from QML or other non-widgets contexts.
// Viewer is passed/stored as QObject* and resolved internally.
class TabPageModel : public QObject
{
  Q_OBJECT
public:
  explicit TabPageModel(QObject* parent = nullptr);
  ~TabPageModel() override;

  // Access the owned Document
  Document& doc();

  // Attach a viewer object (should be an OcctQOpenGLWidgetViewer). The API accepts QObject*
  // to avoid widget class references in the header.
  void setViewerObject(QObject* viewerObject);

  // Create and attach a viewer owned by this model; returns it as QObject*
  // The caller may qobject_cast to the concrete viewer type for embedding.
  QObject* createViewer(QObject* uiParent);

  // Sync viewer bodies and sketches from the Document.
  void syncViewerFromDoc(bool toUpdate = true);

  // Inform listeners that feature list or document tree should refresh.
  // The model itself does not depend on widget implementations.
  void refreshFeatureList();
  void refreshDocumentTree();

  // Selection helper: select feature's AIS body in viewer
  void selectFeatureInViewer(const Handle(Feature)& f);

  // Move tool integration routed via the viewer manipulator
  void activateMove();
  void confirmMove();
  void cancelMove();

signals:
  void featureListChanged();
  void documentTreeChanged();

private:
  // Non-owning pointer to the viewer as QObject; resolved to concrete type in .cpp
  QObject* m_viewerObj = nullptr;
  std::unique_ptr<Document> m_doc;

  // Mappings: Feature <-> AIS body, SketchId -> AIS handle
  TColStd_IndexedDataMapOfTransientTransient m_featureToBody;
  TColStd_IndexedDataMapOfTransientTransient m_bodyToFeature;
  std::unordered_map<DocumentItem::Id, Handle(AIS_Shape)> m_sketchToHandle;

  // Connection used to track manipulator confirmation lifecycle
  QMetaObject::Connection m_connManipFinished;
};
