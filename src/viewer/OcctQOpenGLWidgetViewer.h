#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>
#include <QVector3D>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QTimer>

#include <AIS_InteractiveContext.hxx>
#include <V3d_Viewer.hxx>
#include <V3d_View.hxx>
#include <Graphic3d_Camera.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <AIS_ViewCube.hxx>
#include <AIS_Shape.hxx>
#include <TopoDS_Shape.hxx>

class OcctQOpenGLWidgetViewer : public QOpenGLWidget, protected QOpenGLFunctions
{
  Q_OBJECT

public:
  explicit OcctQOpenGLWidgetViewer(QWidget* parent = nullptr);
  ~OcctQOpenGLWidgetViewer();

  // Основные методы для работы с 3D сценой
  void displayShape(const TopoDS_Shape& shape);
  void clearScene();
  void fitAll();
  void resetView();
  
  // Методы для управления камерой
  void setViewOrientation(int orientation);
  void setViewMode(int mode);
  
  // Методы для ViewCube
  void showViewCube(bool show);
  void setViewCubeSize(int size);
  
  // Методы для генерации миниатюр
  QImage generateThumbnail(int width = 256, int height = 256);

signals:
  void viewChanged();
  void selectionChanged();

protected:
  // QOpenGLWidget переопределения
  void initializeGL() override;
  void paintGL() override;
  void resizeGL(int width, int height) override;
  
  // Обработка событий мыши и клавиатуры
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;

private:
  void initializeOpenCascade();
  void updateProjection();
  void handleMouseRotation(QMouseEvent* event);
  void handleMousePan(QMouseEvent* event);
  void handleMouseZoom(QWheelEvent* event);
  
  // OpenCascade объекты
  Handle(OpenGl_GraphicDriver) m_graphicDriver;
  Handle(V3d_Viewer) m_viewer;
  Handle(V3d_View) m_view;
  Handle(AIS_InteractiveContext) m_context;
  Handle(AIS_ViewCube) m_viewCube;
  
  // Состояние мыши
  QPoint m_lastMousePos;
  bool m_isRotating;
  bool m_isPanning;
  
  // Размеры окна
  int m_width;
  int m_height;
  
  // Флаги
  bool m_viewCubeVisible;
  int m_viewCubeSize;
};
