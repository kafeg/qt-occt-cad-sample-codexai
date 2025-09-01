// Per-tab container: owns a Document and embeds a Viewer
#pragma once

#include <Standard_WarningsDisable.hxx>
#include <QWidget>
#include <Standard_WarningsRestore.hxx>

#include <TColStd_IndexedDataMapOfTransientTransient.hxx>
#include <memory>

class Document;
class OcctQOpenGLWidgetViewer;

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

private:
  OcctQOpenGLWidgetViewer*                 m_viewer = nullptr; // OCCT viewer widget
  std::unique_ptr<Document>                m_doc;              // model document
  TColStd_IndexedDataMapOfTransientTransient m_featureToBody;  // feature -> body
  TColStd_IndexedDataMapOfTransientTransient m_bodyToFeature;  // body -> feature
};
