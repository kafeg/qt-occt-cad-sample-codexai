// QML-based application entry function (no main here).
// Provides runQmlApp() so main.cpp can switch between widget and QML modes.

#include <Standard_WarningsDisable.hxx>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QSurfaceFormat>
#include <QQuickStyle>
#include <QScreen>
#include <QResource>
#include <QDir>
#include <QtQml/qqml.h>

#include "viewer/OcctQmlViewer.h"
#include <Standard_WarningsRestore.hxx>

#include <Standard_Version.hxx>

int runQmlApp(int argc, char** argv)
{
  QGuiApplication app(argc, argv);

  QCoreApplication::setApplicationName("Parametric CAD Skeleton (QML)");
  QCoreApplication::setOrganizationName("OpenCASCADE");
  QCoreApplication::setApplicationVersion(OCC_VERSION_STRING_EXT);

#ifdef __APPLE__
  // macOS: force a Core Profile GL context for OCCT 3D viewer
  const bool     isCoreProfile = true;
  QSurfaceFormat glfmt;
  glfmt.setDepthBufferSize(24);
  glfmt.setStencilBufferSize(8);
  if (isCoreProfile) glfmt.setVersion(4, 5);
  glfmt.setProfile(isCoreProfile ? QSurfaceFormat::CoreProfile : QSurfaceFormat::CompatibilityProfile);
  QSurfaceFormat::setDefaultFormat(glfmt);
#endif

  // Use a consistent, desktop-friendly style
  QQuickStyle::setStyle("Fusion");

  QQmlApplicationEngine engine;
  // Help the engine locate Qt's QML modules when running from the build tree.
  // vcpkg installs them under "vcpkg_installed/<triplet>/Qt6/qml".
  const QString appDir = QCoreApplication::applicationDirPath();
  const QString root   = appDir + "/../vcpkg_installed";
  const QStringList candidates = {
      root + "/arm64-osx/Qt6/qml",
      root + "/arm64-osx/qml",
      root + "/x64-osx/Qt6/qml",
      root + "/x64-osx/qml",
      root + "/debug/Qt6/qml",
      root + "/Qt6/qml"
  };
  for (const QString& p : candidates) {
    if (QDir(p).exists()) engine.addImportPath(p);
  }
  // Also respect environment overrides if the user has them
  const QByteArray envQml = qgetenv("QML2_IMPORT_PATH");
  if (!envQml.isEmpty()) engine.addImportPath(QString::fromLocal8Bit(envQml));
  // Ensure resources from the ui static library are registered
  Q_INIT_RESOURCE(qml);
  const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
  QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app,
                   [url](QObject* obj, const QUrl& objUrl) {
                     if (!obj && url == objUrl) QCoreApplication::exit(-1);
                   }, Qt::QueuedConnection);

  // Register the custom viewer type explicitly when linking statically.
  qmlRegisterType<OcctQmlViewer>("Occt.Viewer", 1, 0, "OcctViewer");

  engine.load(url);
  if (engine.rootObjects().isEmpty()) return -1;
  return app.exec();
}
