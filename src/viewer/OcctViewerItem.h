#pragma once

#include <QQuickItem>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>
#include <QVector3D>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QTimer>
#include <QImage>

#include <AIS_InteractiveContext.hxx>
#include <V3d_Viewer.hxx>
#include <V3d_View.hxx>
#include <Graphic3d_Camera.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <AIS_ViewCube.hxx>
#include <AIS_Shape.hxx>
#include <TopoDS_Shape.hxx>

class OcctWindowWrapper;

class OcctViewerItem : public QQuickItem
{
  Q_OBJECT
  Q_PROPERTY(bool viewCubeVisible READ viewCubeVisible WRITE setViewCubeVisible NOTIFY viewCubeVisibleChanged)
  Q_PROPERTY(int viewCubeSize READ viewCubeSize WRITE setViewCubeSize NOTIFY viewCubeSizeChanged)
  Q_PROPERTY(int viewMode READ viewMode WRITE setViewMode NOTIFY viewModeChanged)
  Q_PROPERTY(int viewOrientation READ viewOrientation WRITE setViewOrientation NOTIFY viewOrientationChanged)

public:
  explicit OcctViewerItem(QQuickItem* parent = nullptr);
  ~OcctViewerItem();

  // QML свойства
  bool viewCubeVisible() const { return m_viewCubeVisible; }
  void setViewCubeVisible(bool visible);
  
  int viewCubeSize() const { return m_viewCubeSize; }
  void setViewCubeSize(int size);
  
  int viewMode() const { return m_viewMode; }
  void setViewMode(int mode);
  
  int viewOrientation() const { return m_viewOrientation; }
  void setViewOrientation(int orientation);

  // QML методы
  Q_INVOKABLE void displayShape(const QVariant& shapeData);
  Q_INVOKABLE void clearScene();
  Q_INVOKABLE void fitAll();
  Q_INVOKABLE void resetView();
  Q_INVOKABLE QImage generateThumbnail(int width = 256, int height = 256);
  
  // Тестовые методы
  Q_INVOKABLE void displayTestBox();
  Q_INVOKABLE void displayTestCylinder();
  Q_INVOKABLE void displayTestSphere();

signals:
  void viewCubeVisibleChanged();
  void viewCubeSizeChanged();
  void viewModeChanged();
  void viewOrientationChanged();
  void viewChanged();
  void selectionChanged();

protected:
  QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* updatePaintNodeData) override;
  void geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry) override;
  void componentComplete() override;
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
  OcctWindowWrapper* m_windowWrapper;
  Handle(V3d_View) m_view;
  Handle(AIS_InteractiveContext) m_context;
  Handle(AIS_ViewCube) m_viewCube;
  
  // Состояние мыши
  QPoint m_lastMousePos;
  bool m_isRotating;
  bool m_isPanning;
  
  // Размеры
  int m_width;
  int m_height;
  
  // Свойства
  bool m_viewCubeVisible;
  int m_viewCubeSize;
  int m_viewMode;
  int m_viewOrientation;
  
  // Флаги инициализации
  bool m_initialized;
  bool m_needsUpdate;
};
