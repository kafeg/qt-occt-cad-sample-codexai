#include "ViewerTypes.h"
#include "OcctViewerItem.h"

void registerViewerTypes()
{
  // Регистрация OcctViewerItem как QML типа
  qmlRegisterType<OcctViewerItem>("VibeCAD.Viewer", 1, 0, "OcctViewer");
}
