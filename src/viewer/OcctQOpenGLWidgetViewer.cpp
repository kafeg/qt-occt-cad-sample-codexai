#include "OcctQOpenGLWidgetViewer.h"

#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QPainter>
#include <QDebug>

#include <OpenGl_GraphicDriver.hxx>
#include <V3d_Viewer.hxx>
#include <V3d_View.hxx>
#include <AIS_InteractiveContext.hxx>
#include <AIS_ViewCube.hxx>
#include <AIS_Shape.hxx>
#include <Graphic3d_Camera.hxx>
#include <V3d_TypeOfOrientation.hxx>
#include <V3d_TypeOfView.hxx>
#include <AIS_DisplayMode.hxx>
#include <Prs3d_Drawer.hxx>
#include <Graphic3d_MaterialAspect.hxx>

OcctQOpenGLWidgetViewer::OcctQOpenGLWidgetViewer(QWidget* parent)
  : QOpenGLWidget(parent)
  , m_isRotating(false)
  , m_isPanning(false)
  , m_width(800)
  , m_height(600)
  , m_viewCubeVisible(true)
  , m_viewCubeSize(100)
{
  setMouseTracking(true);
  setFocusPolicy(Qt::StrongFocus);
}

OcctQOpenGLWidgetViewer::~OcctQOpenGLWidgetViewer()
{
  makeCurrent();
  // OpenCascade автоматически очистит ресурсы
  doneCurrent();
}

void OcctQOpenGLWidgetViewer::initializeGL()
{
  initializeOpenGLFunctions();
  initializeOpenCascade();
}

void OcctQOpenGLWidgetViewer::initializeOpenCascade()
{
  // Создание графического драйвера
  Handle(Aspect_DisplayConnection) displayConnection;
  m_graphicDriver = new OpenGl_GraphicDriver(displayConnection);
  
  // Создание 3D viewer
  m_viewer = new V3d_Viewer(m_graphicDriver);
  m_viewer->SetDefaultLights();
  m_viewer->SetLightOn();
  
  // Создание view
  m_view = m_viewer->CreateView();
  // Для QOpenGLWidget нужно создать специальный window wrapper
  // Пока что пропускаем SetWindow
  m_view->MustBeResized();
  m_view->TriedronDisplay(Aspect_TOTP_LEFT_LOWER, Quantity_NOC_GOLD, 0.08, V3d_ZBUFFER);
  
  // Создание интерактивного контекста
  m_context = new AIS_InteractiveContext(m_viewer);
  m_context->SetDisplayMode(AIS_Shaded, Standard_True);
  
  // Создание ViewCube
  m_viewCube = new AIS_ViewCube();
  m_viewCube->SetBoxColor(Quantity_NOC_GRAY);
  m_viewCube->SetFixedAnimationLoop(false);
  m_viewCube->SetAutoStartAnimation(true);
  m_context->Display(m_viewCube, Standard_True);
  
  // Настройка камеры
  m_view->SetAt(0, 0, 0);
  m_view->SetEye(100, 100, 100);
  m_view->SetUp(0, 0, 1);
  
  qDebug() << "OpenCascade viewer initialized successfully";
}

void OcctQOpenGLWidgetViewer::paintGL()
{
  if (m_view.IsNull()) {
    return;
  }
  
  // Очистка буфера
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  // Рендеринг OpenCascade сцены
  m_view->Redraw();
}

void OcctQOpenGLWidgetViewer::resizeGL(int width, int height)
{
  m_width = width;
  m_height = height;
  
  if (!m_view.IsNull()) {
    m_view->MustBeResized();
    updateProjection();
  }
}

void OcctQOpenGLWidgetViewer::updateProjection()
{
  if (m_view.IsNull()) {
    return;
  }
  
  // Обновление проекции для нового размера окна
  m_view->MustBeResized();
  m_view->Redraw();
}

void OcctQOpenGLWidgetViewer::displayShape(const TopoDS_Shape& shape)
{
  if (m_context.IsNull() || shape.IsNull()) {
    return;
  }
  
  // Создание AIS_Shape из TopoDS_Shape
  Handle(AIS_Shape) aisShape = new AIS_Shape(shape);
  
  // Отображение формы
  m_context->Display(aisShape, Standard_True);
  
  // Автоматическое масштабирование
  fitAll();
}

void OcctQOpenGLWidgetViewer::clearScene()
{
  if (m_context.IsNull()) {
    return;
  }
  
  // Очистка всех объектов кроме ViewCube
  m_context->RemoveAll(Standard_True);
  
  // Восстановление ViewCube
  if (m_viewCubeVisible && !m_viewCube.IsNull()) {
    m_context->Display(m_viewCube, Standard_True);
  }
}

void OcctQOpenGLWidgetViewer::fitAll()
{
  if (m_view.IsNull()) {
    return;
  }
  
  m_view->FitAll();
  m_view->ZFitAll();
  update();
}

void OcctQOpenGLWidgetViewer::resetView()
{
  if (m_view.IsNull()) {
    return;
  }
  
  m_view->Reset();
  fitAll();
}

void OcctQOpenGLWidgetViewer::setViewOrientation(int orientation)
{
  if (m_view.IsNull()) {
    return;
  }
  
  V3d_TypeOfOrientation occtOrientation = V3d_XposYnegZpos;
  
  switch (orientation) {
    case 0: occtOrientation = V3d_XposYnegZpos; break; // Изометрия
    case 1: occtOrientation = V3d_Xpos; break;         // Спереди
    case 2: occtOrientation = V3d_Ypos; break;         // Справа
    case 3: occtOrientation = V3d_Zpos; break;         // Сверху
    case 4: occtOrientation = V3d_Xneg; break;         // Сзади
    case 5: occtOrientation = V3d_Yneg; break;         // Слева
    case 6: occtOrientation = V3d_Zneg; break;         // Снизу
    default: occtOrientation = V3d_XposYnegZpos; break;
  }
  
  m_view->SetProj(occtOrientation);
  update();
}

void OcctQOpenGLWidgetViewer::setViewMode(int mode)
{
  if (m_view.IsNull()) {
    return;
  }
  
  switch (mode) {
    case 0: m_view->SetComputedMode(Standard_False); break; // Wireframe
    case 1: m_view->SetComputedMode(Standard_True); break;  // Shaded
    default: break;
  }
  
  update();
}

void OcctQOpenGLWidgetViewer::showViewCube(bool show)
{
  m_viewCubeVisible = show;
  
  if (m_context.IsNull() || m_viewCube.IsNull()) {
    return;
  }
  
  if (show) {
    m_context->Display(m_viewCube, Standard_True);
  } else {
    m_context->Remove(m_viewCube, Standard_True);
  }
  
  update();
}

void OcctQOpenGLWidgetViewer::setViewCubeSize(int size)
{
  m_viewCubeSize = size;
  
  if (m_viewCube.IsNull()) {
    return;
  }
  
  // ViewCube размер настраивается через его параметры
  // Здесь можно добавить настройку размера если необходимо
  update();
}

QImage OcctQOpenGLWidgetViewer::generateThumbnail(int width, int height)
{
  if (m_view.IsNull()) {
    return QImage();
  }
  
  // Создание off-screen буфера для рендеринга миниатюры
  QOpenGLFramebufferObject fbo(width, height);
  fbo.bind();
  
  // Временное изменение размера view
  m_view->MustBeResized();
  m_view->Redraw();
  
  // Получение изображения из буфера
  QImage image = fbo.toImage();
  
  fbo.release();
  
  return image;
}

void OcctQOpenGLWidgetViewer::mousePressEvent(QMouseEvent* event)
{
  if (m_view.IsNull()) {
    return;
  }
  
  m_lastMousePos = event->position().toPoint();
  
  if (event->button() == Qt::LeftButton) {
    m_isRotating = true;
  } else if (event->button() == Qt::MiddleButton) {
    m_isPanning = true;
  }
  
  // Обработка выбора объектов
  if (event->button() == Qt::LeftButton && !m_isRotating) {
    QPointF pos = event->position();
    m_context->MoveTo(pos.x(), pos.y(), m_view, Standard_True);
    m_context->SelectDetected(AIS_SelectionScheme_Replace);
  }
}

void OcctQOpenGLWidgetViewer::mouseMoveEvent(QMouseEvent* event)
{
  if (m_view.IsNull()) {
    return;
  }
  
  if (m_isRotating) {
    handleMouseRotation(event);
  } else if (m_isPanning) {
    handleMousePan(event);
  } else {
    // Подсветка объектов при наведении
    QPointF pos = event->position();
    m_context->MoveTo(pos.x(), pos.y(), m_view, Standard_True);
  }
}

void OcctQOpenGLWidgetViewer::mouseReleaseEvent(QMouseEvent* event)
{
  m_isRotating = false;
  m_isPanning = false;
}

void OcctQOpenGLWidgetViewer::wheelEvent(QWheelEvent* event)
{
  if (m_view.IsNull()) {
    return;
  }
  
  handleMouseZoom(event);
}

void OcctQOpenGLWidgetViewer::keyPressEvent(QKeyEvent* event)
{
  if (m_view.IsNull()) {
    return;
  }
  
  switch (event->key()) {
    case Qt::Key_F:
      fitAll();
      break;
    case Qt::Key_R:
      resetView();
      break;
    case Qt::Key_1:
      setViewOrientation(1); // Спереди
      break;
    case Qt::Key_2:
      setViewOrientation(2); // Справа
      break;
    case Qt::Key_3:
      setViewOrientation(3); // Сверху
      break;
    case Qt::Key_4:
      setViewOrientation(0); // Изометрия
      break;
    default:
      QOpenGLWidget::keyPressEvent(event);
      break;
  }
}

void OcctQOpenGLWidgetViewer::handleMouseRotation(QMouseEvent* event)
{
  if (m_view.IsNull()) {
    return;
  }
  
  QPointF pos = event->position();
  int dx = pos.x() - m_lastMousePos.x();
  int dy = pos.y() - m_lastMousePos.y();
  
  m_view->Rotation(dx, dy);
  m_lastMousePos = pos.toPoint();
  
  update();
}

void OcctQOpenGLWidgetViewer::handleMousePan(QMouseEvent* event)
{
  if (m_view.IsNull()) {
    return;
  }
  
  QPointF pos = event->position();
  int dx = pos.x() - m_lastMousePos.x();
  int dy = pos.y() - m_lastMousePos.y();
  
  m_view->Pan(dx, -dy);
  m_lastMousePos = pos.toPoint();
  
  update();
}

void OcctQOpenGLWidgetViewer::handleMouseZoom(QWheelEvent* event)
{
  if (m_view.IsNull()) {
    return;
  }
  
  double delta = event->angleDelta().y() / 120.0;
  double scale = 1.0 + delta * 0.1;
  
  m_view->SetZoom(scale);
  update();
}
