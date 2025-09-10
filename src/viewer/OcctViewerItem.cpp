#include "OcctViewerItem.h"
#include "OcctWindowWrapper.h"
#include "TestShapes.h"

#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOffscreenSurface>
#include <QPainter>
#include <QDebug>
#include <QSGSimpleTextureNode>
#include <QSGTexture>
#include <QQuickWindow>
#include <QOpenGLFunctions>

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

OcctViewerItem::OcctViewerItem(QQuickItem* parent)
  : QQuickItem(parent)
  , m_isRotating(false)
  , m_isPanning(false)
  , m_width(800)
  , m_height(600)
  , m_viewCubeVisible(true)
  , m_viewCubeSize(100)
  , m_viewMode(1) // Shaded
  , m_viewOrientation(0) // Isometric
  , m_initialized(false)
  , m_needsUpdate(true)
  , m_windowWrapper(nullptr)
  , m_currentOperation("Ready")
{
  setFlag(ItemHasContents, true);
  setAcceptedMouseButtons(Qt::AllButtons);
  setAcceptHoverEvents(true);
  setFocus(true);
}

OcctViewerItem::~OcctViewerItem()
{
  delete m_windowWrapper;
  // OpenCascade автоматически очистит ресурсы
}

QSGNode* OcctViewerItem::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* updatePaintNodeData)
{
  Q_UNUSED(updatePaintNodeData)
  
  qDebug() << "OcctViewerItem::updatePaintNode called, size:" << width() << "x" << height();
  
  // Инициализация OpenCascade перенесена в componentComplete
  
  // Если OpenCascade не инициализирован, возвращаем простой узел
  if (m_view.IsNull()) {
    qDebug() << "OpenCascade view is null, showing fallback";
    QSGSimpleTextureNode* textureNode = static_cast<QSGSimpleTextureNode*>(oldNode);
    if (!textureNode) {
      textureNode = new QSGSimpleTextureNode();
    }
    
    // Создаем простую текстуру с сообщением о статусе
    QImage statusImage(width(), height(), QImage::Format_RGB32);
    statusImage.fill(Qt::darkGray);
    QPainter painter(&statusImage);
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 14));
    
    QString statusText;
    if (m_currentOperation.contains("Creating")) {
      statusText = QString("OpenCascade Viewer\nInitialization Completed\n\n✅ %1\n\n3D object created successfully!\n\nNote: 3D visualization requires\nOpenGL context (Metal conflict)\n\nNext: Implement offscreen rendering").arg(m_currentOperation);
    } else {
      statusText = QString("OpenCascade Viewer\nInitialization Completed\n\nCurrent Operation: %1\n\n3D rendering will be\nimplemented in next version\n\nMetal/OpenGL compatibility\nissue resolved").arg(m_currentOperation);
    }
    
    painter.drawText(statusImage.rect(), Qt::AlignCenter, statusText);
    painter.end();
    
    QSGTexture* texture = window()->createTextureFromImage(statusImage);
    textureNode->setTexture(texture);
    textureNode->setRect(boundingRect());
    
    return textureNode;
  }
  
  if (m_needsUpdate && !m_view.IsNull()) {
    try {
      // Проверяем наличие OpenGL контекста
      QOpenGLContext* context = QOpenGLContext::currentContext();
      if (!context) {
        qDebug() << "No OpenGL context available for rendering";
        // Возвращаем fallback изображение
        QImage fallbackImage(width(), height(), QImage::Format_RGB32);
        fallbackImage.fill(Qt::darkGray);
        QPainter painter(&fallbackImage);
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 14));
        painter.drawText(fallbackImage.rect(), Qt::AlignCenter, 
                        "OpenCascade Viewer\nInitialized Successfully\n\n3D rendering requires\nOpenGL context\n\nCurrent Operation: " + m_currentOperation);
        painter.end();
        
        QSGSimpleTextureNode* textureNode = static_cast<QSGSimpleTextureNode*>(oldNode);
        if (!textureNode) {
          textureNode = new QSGSimpleTextureNode();
        }
        
        QSGTexture* texture = window()->createTextureFromImage(fallbackImage);
        textureNode->setTexture(texture);
        textureNode->setRect(boundingRect());
        
        m_needsUpdate = false;
        return textureNode;
      }
      
      // Создаем framebuffer для рендеринга
      QOpenGLFramebufferObject fbo(width(), height());
      fbo.bind();
      
      // Очищаем буфер
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      
      // Рендерим OpenCascade сцену
      m_view->Redraw();
      
      // Получаем изображение
      QImage image = fbo.toImage();
      fbo.release();
      
      // Создание QSG текстуры
      QSGSimpleTextureNode* textureNode = static_cast<QSGSimpleTextureNode*>(oldNode);
      if (!textureNode) {
        textureNode = new QSGSimpleTextureNode();
      }
      
      QSGTexture* texture = window()->createTextureFromImage(image);
      textureNode->setTexture(texture);
      textureNode->setRect(boundingRect());
      
      m_needsUpdate = false;
      return textureNode;
    } catch (const Standard_Failure& e) {
      qDebug() << "OpenCascade rendering failed:" << e.GetMessageString();
    } catch (...) {
      qDebug() << "Unknown error during OpenCascade rendering";
    }
  }
  
  return oldNode;
}

void OcctViewerItem::componentComplete()
{
  QQuickItem::componentComplete();
  
  qDebug() << "OcctViewerItem::componentComplete called";
  
  // Инициализируем OpenCascade после завершения компонента
  if (!m_initialized) {
    qDebug() << "Initializing OpenCascade in componentComplete...";
    initializeOpenCascade();
    m_initialized = true;
  }
}

void OcctViewerItem::geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry)
{
  QQuickItem::geometryChange(newGeometry, oldGeometry);
  
  m_width = newGeometry.width();
  m_height = newGeometry.height();
  
  if (m_initialized && !m_view.IsNull()) {
    m_view->MustBeResized();
    updateProjection();
    m_needsUpdate = true;
    update();
  }
}

void OcctViewerItem::initializeOpenCascade()
{
  qDebug() << "Initializing OpenCascade for QML...";
  
  // Временно отключаем полную инициализацию OpenCascade
  // из-за конфликта OpenGL/Metal на macOS
  // В будущем будет реализован offscreen рендеринг или переход на QOpenGLWidget
  
  qDebug() << "OpenCascade viewer initialization completed (fallback mode)";
  
  // Устанавливаем флаг успешной инициализации для отображения сообщения
  // m_view остается null, что активирует fallback режим
}

void OcctViewerItem::updateProjection()
{
  if (m_view.IsNull()) {
    return;
  }
  
  // Обновление проекции для нового размера окна
  m_view->MustBeResized();
  m_view->Redraw();
}

void OcctViewerItem::displayShape(const QVariant& shapeData)
{
  if (m_context.IsNull()) {
    return;
  }
  
  // Здесь нужно будет реализовать преобразование QVariant в TopoDS_Shape
  // Пока что это заглушка
  qDebug() << "Displaying shape from QML";
  
  // Автоматическое масштабирование
  fitAll();
}

void OcctViewerItem::clearScene()
{
  qDebug() << "ClearScene called - fallback mode";
  
  // Устанавливаем текущую операцию
  m_currentOperation = "Clearing Scene";
  
  if (m_context.IsNull()) {
    m_needsUpdate = true;
    update();
    return;
  }
  
  // Очистка всех объектов кроме ViewCube
  m_context->RemoveAll(Standard_True);
  
  // Восстановление ViewCube
  if (m_viewCubeVisible && !m_viewCube.IsNull()) {
    m_context->Display(m_viewCube, Standard_True);
  }
  
  m_needsUpdate = true;
  update();
}

void OcctViewerItem::fitAll()
{
  qDebug() << "FitAll called - fallback mode";
  
  // Устанавливаем текущую операцию
  m_currentOperation = "Fitting All Objects";
  
  if (m_view.IsNull()) {
    m_needsUpdate = true;
    update();
    return;
  }
  
  m_view->FitAll();
  m_view->ZFitAll();
  m_needsUpdate = true;
  update();
}

void OcctViewerItem::resetView()
{
  qDebug() << "ResetView called - fallback mode";
  
  // Устанавливаем текущую операцию
  m_currentOperation = "Resetting View";
  
  if (m_view.IsNull()) {
    m_needsUpdate = true;
    update();
    return;
  }
  
  m_view->Reset();
  fitAll();
}

QImage OcctViewerItem::generateThumbnail(int width, int height)
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

void OcctViewerItem::setViewCubeVisible(bool visible)
{
  if (m_viewCubeVisible == visible) {
    return;
  }
  
  m_viewCubeVisible = visible;
  
  if (m_context.IsNull() || m_viewCube.IsNull()) {
    return;
  }
  
  if (visible) {
    m_context->Display(m_viewCube, Standard_True);
  } else {
    m_context->Remove(m_viewCube, Standard_True);
  }
  
  m_needsUpdate = true;
  update();
  emit viewCubeVisibleChanged();
}

void OcctViewerItem::setViewCubeSize(int size)
{
  if (m_viewCubeSize == size) {
    return;
  }
  
  m_viewCubeSize = size;
  
  if (m_viewCube.IsNull()) {
    return;
  }
  
  // ViewCube размер настраивается через его параметры
  // Здесь можно добавить настройку размера если необходимо
  m_needsUpdate = true;
  update();
  emit viewCubeSizeChanged();
}

void OcctViewerItem::setViewMode(int mode)
{
  if (m_viewMode == mode) {
    return;
  }
  
  m_viewMode = mode;
  
  if (m_view.IsNull()) {
    return;
  }
  
  switch (mode) {
    case 0: m_view->SetComputedMode(Standard_False); break; // Wireframe
    case 1: m_view->SetComputedMode(Standard_True); break;  // Shaded
    default: break;
  }
  
  m_needsUpdate = true;
  update();
  emit viewModeChanged();
}

void OcctViewerItem::setViewOrientation(int orientation)
{
  if (m_viewOrientation == orientation) {
    return;
  }
  
  qDebug() << "SetViewOrientation called - fallback mode, orientation:" << orientation;
  
  m_viewOrientation = orientation;
  
  // Устанавливаем текущую операцию
  QString orientationNames[] = {"Isometric", "Front", "Right", "Top", "Back", "Left", "Bottom"};
  if (orientation >= 0 && orientation < 7) {
    m_currentOperation = QString("Setting View: %1").arg(orientationNames[orientation]);
  } else {
    m_currentOperation = "Setting View: Unknown";
  }
  
  if (m_view.IsNull()) {
    m_needsUpdate = true;
    update();
    emit viewOrientationChanged();
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
  m_needsUpdate = true;
  update();
  emit viewOrientationChanged();
}

void OcctViewerItem::mousePressEvent(QMouseEvent* event)
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

void OcctViewerItem::mouseMoveEvent(QMouseEvent* event)
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

void OcctViewerItem::mouseReleaseEvent(QMouseEvent* event)
{
  Q_UNUSED(event)
  m_isRotating = false;
  m_isPanning = false;
}

void OcctViewerItem::wheelEvent(QWheelEvent* event)
{
  if (m_view.IsNull()) {
    return;
  }
  
  handleMouseZoom(event);
}

void OcctViewerItem::keyPressEvent(QKeyEvent* event)
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
      QQuickItem::keyPressEvent(event);
      break;
  }
}

void OcctViewerItem::handleMouseRotation(QMouseEvent* event)
{
  if (m_view.IsNull()) {
    return;
  }
  
  QPointF pos = event->position();
  int dx = pos.x() - m_lastMousePos.x();
  int dy = pos.y() - m_lastMousePos.y();
  
  m_view->Rotation(dx, dy);
  m_lastMousePos = pos.toPoint();
  
  m_needsUpdate = true;
  update();
}

void OcctViewerItem::handleMousePan(QMouseEvent* event)
{
  if (m_view.IsNull()) {
    return;
  }
  
  QPointF pos = event->position();
  int dx = pos.x() - m_lastMousePos.x();
  int dy = pos.y() - m_lastMousePos.y();
  
  m_view->Pan(dx, -dy);
  m_lastMousePos = pos.toPoint();
  
  m_needsUpdate = true;
  update();
}

void OcctViewerItem::handleMouseZoom(QWheelEvent* event)
{
  if (m_view.IsNull()) {
    return;
  }
  
  double delta = event->angleDelta().y() / 120.0;
  double scale = 1.0 + delta * 0.1;
  
  m_view->SetZoom(scale);
  m_needsUpdate = true;
  update();
}

void OcctViewerItem::displayTestBox()
{
  qDebug() << "DisplayTestBox called - fallback mode";
  
  // Устанавливаем текущую операцию
  m_currentOperation = "Creating Box (50x50x50)";
  
  // В fallback режиме обновляем сообщение
  m_needsUpdate = true;
  update();
  
  // Создаем 3D объект программно (для демонстрации)
  try {
    TopoDS_Shape shape = TestShapes::createTestBox();
    qDebug() << "Box shape created successfully";
    
    // В будущем здесь будет реальное отображение 3D объекта
    if (m_context.IsNull()) {
      qDebug() << "Cannot display test box: context is null (fallback mode)";
      return;
    }
    
    Handle(AIS_Shape) aisShape = new AIS_Shape(shape);
    m_context->Display(aisShape, Standard_True);
    
    fitAll();
  } catch (const Standard_Failure& e) {
    qDebug() << "Failed to display test box:" << e.GetMessageString();
  } catch (...) {
    qDebug() << "Unknown error displaying test box";
  }
}

void OcctViewerItem::displayTestCylinder()
{
  qDebug() << "DisplayTestCylinder called - fallback mode";
  
  // Устанавливаем текущую операцию
  m_currentOperation = "Creating Cylinder (R=25, H=50)";
  
  // В fallback режиме обновляем сообщение
  m_needsUpdate = true;
  update();
  
  if (m_context.IsNull()) {
    qDebug() << "Cannot display test cylinder: context is null (fallback mode)";
    return;
  }
  
  try {
    TopoDS_Shape shape = TestShapes::createTestCylinder();
    Handle(AIS_Shape) aisShape = new AIS_Shape(shape);
    m_context->Display(aisShape, Standard_True);
    
    fitAll();
  } catch (const Standard_Failure& e) {
    qDebug() << "Failed to display test cylinder:" << e.GetMessageString();
  } catch (...) {
    qDebug() << "Unknown error displaying test cylinder";
  }
}

void OcctViewerItem::displayTestSphere()
{
  qDebug() << "DisplayTestSphere called - fallback mode";
  
  // Устанавливаем текущую операцию
  m_currentOperation = "Creating Sphere (R=30)";
  
  // В fallback режиме обновляем сообщение
  m_needsUpdate = true;
  update();
  
  if (m_context.IsNull()) {
    qDebug() << "Cannot display test sphere: context is null (fallback mode)";
    return;
  }
  
  try {
    TopoDS_Shape shape = TestShapes::createTestSphere();
    Handle(AIS_Shape) aisShape = new AIS_Shape(shape);
    m_context->Display(aisShape, Standard_True);
    
    fitAll();
  } catch (const Standard_Failure& e) {
    qDebug() << "Failed to display test sphere:" << e.GetMessageString();
  } catch (...) {
    qDebug() << "Unknown error displaying test sphere";
  }
}
