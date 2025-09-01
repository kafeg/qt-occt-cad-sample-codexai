// Main application window using reusable OCCT QOpenGLWidget viewer
#pragma once

#include <Standard_WarningsDisable.hxx>
#include <QMainWindow>
#include <Standard_WarningsRestore.hxx>
#include <TColStd_IndexedDataMapOfTransientTransient.hxx>
#include <AIS_Shape.hxx>
#include <Feature.h>
#include <memory>

class Document;
class OcctQOpenGLWidgetViewer;
class QTabWidget;
class TabPage;

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow();
  ~MainWindow();

private:
  void createMenuBar();
  void createToolBar();
  void addBox();
  void addCylinder();
  void syncViewerFromDoc(bool toUpdate = true);
  void clearAll();
  void addSample();
  void addNewTab();

  TabPage* currentPage() const;

private:
  QTabWidget* m_tabs = nullptr;
};
