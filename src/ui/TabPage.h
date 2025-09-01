// Per-tab container: owns a Document and embeds a Viewer
#pragma once

#include <Standard_WarningsDisable.hxx>
#include <QWidget>
#include <Standard_WarningsRestore.hxx>

#include <TColStd_IndexedDataMapOfTransientTransient.hxx>
#include <memory>
#include <vector>

class Document;
class OcctQOpenGLWidgetViewer;
class Sketch;

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

  // Simple sketch registry for the page (prototype): list of available sketches
  std::vector<std::shared_ptr<Sketch>>& sketches() { return m_sketches; }

private:
  OcctQOpenGLWidgetViewer*                 m_viewer = nullptr; // OCCT viewer widget
  std::unique_ptr<Document>                m_doc;              // model document
  TColStd_IndexedDataMapOfTransientTransient m_featureToBody;  // feature -> body
  TColStd_IndexedDataMapOfTransientTransient m_bodyToFeature;  // body -> feature
  std::vector<std::shared_ptr<Sketch>>     m_sketches;         // available sketches
};
