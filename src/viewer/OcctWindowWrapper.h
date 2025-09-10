#pragma once

#include <QWindow>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOffscreenSurface>

#include <Aspect_Window.hxx>
#include <Aspect_DisplayConnection.hxx>

class OcctWindowWrapper : public Aspect_Window, protected QOpenGLFunctions
{
public:
    OcctWindowWrapper(QWindow* window);
    virtual ~OcctWindowWrapper();

    // Aspect_Window interface
    virtual Standard_Boolean IsMapped() const override;
    virtual Standard_Boolean DoMapping() const override;
    virtual void Map() const override;
    virtual void Unmap() const override;
    virtual Aspect_TypeOfResize DoResize() override;
    virtual Standard_Real Ratio() const override;
    virtual void Position(Standard_Integer& X1, Standard_Integer& Y1, Standard_Integer& X2, Standard_Integer& Y2) const override;
    virtual void Size(Standard_Integer& Width, Standard_Integer& Height) const override;
    virtual Aspect_Drawable NativeHandle() const override;
    virtual Aspect_Drawable NativeParentHandle() const override;
    virtual Aspect_FBConfig NativeFBConfig() const override;
    virtual void SetTitle(const TCollection_AsciiString&) override;
    virtual void InvalidateContent(const Handle(Aspect_DisplayConnection)&) override;

    // Custom methods
    void setSize(int width, int height);
    QSize size() const { return m_size; }
    QWindow* window() const { return m_window; }

private:
    QWindow* m_window;
    QSize m_size;
    QOpenGLContext* m_context;
    QOffscreenSurface* m_offscreenSurface;
};
