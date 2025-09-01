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

// Main app window: manages tabs, menu/toolbars, and sync to viewer
class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow();
  ~MainWindow();

private:
  void createMenuBar();             // File/Test menus and actions
  void createToolBar();             // Toolbar with background slider and test actions
  void addBox();                    // Open dialog and add BoxFeature
  void addCylinder();               // Open dialog and add CylinderFeature
  void syncViewerFromDoc(bool toUpdate = true); // Rebuild AIS bodies from Document
  void clearAll();                  // Clear document and viewer
  void addSample();                 // Add arranged sample primitives (test-only helpers)
  void addNewTab();                 // Add a new tab page

  TabPage* currentPage() const;

private:
  QTabWidget* m_tabs = nullptr; // App tabs; each holds a TabPage
};
