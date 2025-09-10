#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QLibraryInfo>
#include <QScreen>
#include "viewer/ViewerTypes.h"
#include <QQuickWindow>
#include <QSurfaceFormat>
#include <QCoreApplication>

int main(int argc, char* argv[]) {
    // Force Qt Quick to use OpenGL (required for QQuickFramebufferObject-based OCCT viewer)
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
#if defined(Q_OS_MAC)
    // macOS prefers 4.1 Core Profile
    QSurfaceFormat fmt;
    fmt.setRenderableType(QSurfaceFormat::OpenGL);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    fmt.setVersion(4, 1);
    fmt.setDepthBufferSize(24);
    fmt.setStencilBufferSize(8);
    QSurfaceFormat::setDefaultFormat(fmt);
#endif
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    QGuiApplication app(argc, argv);

    // Exit gracefully if no screens are available (headless environment)
    if (QGuiApplication::screens().isEmpty()) {
        qCritical("No screens available. Are you running headless?");
        return 2;
    }

    QQmlApplicationEngine engine;
    // Help the engine find Qt's QML modules when running from the build tree
    engine.addImportPath(QLibraryInfo::path(QLibraryInfo::QmlImportsPath));
    
    // Register custom QML types
    registerViewerTypes();
    const QUrl url(QStringLiteral("qrc:/ui/qml/main.qml"));
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [url](QObject* obj, const QUrl& objUrl) {
            if (!obj && url == objUrl) {
                QCoreApplication::exit(-1);
            }
        },
        Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
