#include "OcctWindowWrapper.h"
#include <QDebug>

OcctWindowWrapper::OcctWindowWrapper(QWindow* window)
    : m_window(window)
    , m_size(800, 600)
    , m_context(nullptr)
    , m_offscreenSurface(nullptr)
{
    if (m_window) {
        m_size = m_window->size();
        qDebug() << "OcctWindowWrapper created for window size:" << m_size;
    } else {
        qDebug() << "OcctWindowWrapper created without window";
    }
    
    // Для QML интеграции не создаем собственный контекст
    // Используем контекст из QML
    m_context = nullptr;
    m_offscreenSurface = nullptr;
}

OcctWindowWrapper::~OcctWindowWrapper()
{
    // Контекст и surface не создаются в конструкторе для QML
    // поэтому не нужно их удалять
    qDebug() << "OcctWindowWrapper destroyed";
}

Standard_Boolean OcctWindowWrapper::IsMapped() const
{
    return m_window ? m_window->isVisible() : Standard_True;
}

Standard_Boolean OcctWindowWrapper::DoMapping() const
{
    return Standard_True;
}

void OcctWindowWrapper::Map() const
{
    if (m_window) {
        m_window->show();
    }
}

void OcctWindowWrapper::Unmap() const
{
    if (m_window) {
        m_window->hide();
    }
}

Aspect_TypeOfResize OcctWindowWrapper::DoResize()
{
    return Aspect_TOR_UNKNOWN;
}

Standard_Real OcctWindowWrapper::Ratio() const
{
    if (m_size.height() > 0) {
        return (Standard_Real)m_size.width() / m_size.height();
    }
    return 1.0;
}

void OcctWindowWrapper::Position(Standard_Integer& X1, Standard_Integer& Y1, Standard_Integer& X2, Standard_Integer& Y2) const
{
    if (m_window) {
        QPoint pos = m_window->position();
        X1 = pos.x();
        Y1 = pos.y();
        X2 = pos.x() + m_size.width();
        Y2 = pos.y() + m_size.height();
    } else {
        X1 = Y1 = X2 = Y2 = 0;
    }
}

void OcctWindowWrapper::Size(Standard_Integer& Width, Standard_Integer& Height) const
{
    Width = m_size.width();
    Height = m_size.height();
}

Aspect_Drawable OcctWindowWrapper::NativeHandle() const
{
    if (m_window) {
        return (Aspect_Drawable)m_window->winId();
    }
    return 0;
}

Aspect_Drawable OcctWindowWrapper::NativeParentHandle() const
{
    return 0;
}

Aspect_FBConfig OcctWindowWrapper::NativeFBConfig() const
{
    return 0; // Not implemented for Qt
}

void OcctWindowWrapper::SetTitle(const TCollection_AsciiString& title)
{
    if (m_window) {
        m_window->setTitle(QString::fromUtf8(title.ToCString()));
    }
}

void OcctWindowWrapper::InvalidateContent(const Handle(Aspect_DisplayConnection)&)
{
    if (m_window) {
        m_window->requestUpdate();
    }
}

void OcctWindowWrapper::setSize(int width, int height)
{
    m_size = QSize(width, height);
    qDebug() << "OcctWindowWrapper size set to:" << m_size;
    // Для QML не изменяем размер окна, только внутренний размер
}
