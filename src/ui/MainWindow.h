// Main application window using reusable OCCT QOpenGLWidget viewer
#pragma once

#include <Standard_WarningsDisable.hxx>
#include <QMainWindow>
#include <Standard_WarningsRestore.hxx>

class OcctQOpenGLWidgetViewer;

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow();

private:
  void createMenuBar();
  void createToolBar();

private:
  OcctQOpenGLWidgetViewer* myViewer = nullptr;
};

