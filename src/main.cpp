// Qt application entry point; creates MainWindow and starts the event loop
#include "ui/MainWindow.h"

#include <Standard_WarningsDisable.hxx>
#include <QApplication>
#include <QSurfaceFormat>
#include <QScreen>
#include <QStyle>
#include <Standard_WarningsRestore.hxx>

#include <Standard_Version.hxx>

// Declaration provided by src/main_qml.cpp
int runQmlApp(int argc, char** argv);

int main(int argc, char** argv)
{
  // Runtime switch: --qml or CAD_USE_QML=1 launches the QML UI
  bool useQml = true;//qEnvironmentVariableIsSet("CAD_USE_QML");
  for (int i = 1; i < argc; ++i)
  {
    if (QString::fromLocal8Bit(argv[i]) == "--qml")
    {
      useQml = true;
      break;
    }
  }
  if (useQml) {
    return runQmlApp(argc, argv);
  }

  QApplication app(argc, argv);

  QCoreApplication::setApplicationName("Parametric CAD Skeleton");
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

  MainWindow win;
  // Make initial window larger (1.5x of the hint) and center it
  const QSize s = win.sizeHint();
  win.resize(int(s.width() * 1.3), int(s.height() * 1.3));
  if (QScreen* screen = QGuiApplication::primaryScreen())
  {
    const QRect avail = screen->availableGeometry();
    const QRect geom  = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, win.size(), avail);
    win.setGeometry(geom);
  }
  win.show();
  return app.exec();
}
