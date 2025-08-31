// Main application window using reusable OCCT QOpenGLWidget viewer
#pragma once

#include <Standard_WarningsDisable.hxx>
#include <QMainWindow>
#include <Standard_WarningsRestore.hxx>
#include <TColStd_IndexedDataMapOfTransientTransient.hxx>
#include <AIS_Shape.hxx>
#include <Feature.h>

class Document;
class OcctQOpenGLWidgetViewer;

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow();

private:
  void createMenuBar();
  void createToolBar();
  void addBox();
  void deleteSelected();
  void syncViewerFromDoc(bool toUpdate = true);
  void clearAll();
  void addSample();

private:
  OcctQOpenGLWidgetViewer* myViewer = nullptr;
  Document*                myDoc    = nullptr;
  TColStd_IndexedDataMapOfTransientTransient myFeatureToBody;
  TColStd_IndexedDataMapOfTransientTransient myBodyToFeature;
};
