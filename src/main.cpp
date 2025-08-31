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
  bool           isCoreProfile = true;
  QSurfaceFormat aGlFormat;
  aGlFormat.setDepthBufferSize(24);
  aGlFormat.setStencilBufferSize(8);
  if (isCoreProfile) aGlFormat.setVersion(4, 5);
  aGlFormat.setProfile(isCoreProfile ? QSurfaceFormat::CoreProfile : QSurfaceFormat::CompatibilityProfile);
  QSurfaceFormat::setDefaultFormat(aGlFormat);
#endif

  MainWindow win;
  win.resize(win.sizeHint());
  win.show();
  return app.exec();
}

