// Qt application entry point; creates MainWindow and starts the event loop
#include "ui/MainWindow.h"

#include <Standard_WarningsDisable.hxx>
#include <QApplication>
#include <QSurfaceFormat>
#include <Standard_WarningsRestore.hxx>

#include <Standard_Version.hxx>

int main(int argc, char** argv)
{
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
  win.resize(win.sizeHint());
  win.show();
  return app.exec();
}
