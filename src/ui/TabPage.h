// Per-tab container: owns a Document and embeds a Viewer
#pragma once

#include <Standard_WarningsDisable.hxx>
#include <QWidget>
#include <Standard_WarningsRestore.hxx>

#include <TColStd_IndexedDataMapOfTransientTransient.hxx>
#include <memory>

class Document;
class OcctQOpenGLWidgetViewer;

class TabPage : public QWidget
{
  Q_OBJECT
public:
  explicit TabPage(QWidget* parent = nullptr);
  ~TabPage() override;

  OcctQOpenGLWidgetViewer* viewer() const { return myViewer; }
  Document&                doc()          { return *myDoc; }

  TColStd_IndexedDataMapOfTransientTransient& featureToBody() { return myFeatureToBody; }
  TColStd_IndexedDataMapOfTransientTransient& bodyToFeature() { return myBodyToFeature; }

private:
  OcctQOpenGLWidgetViewer*                 myViewer = nullptr;
  std::unique_ptr<Document>                myDoc;
  TColStd_IndexedDataMapOfTransientTransient myFeatureToBody;
  TColStd_IndexedDataMapOfTransientTransient myBodyToFeature;
};

