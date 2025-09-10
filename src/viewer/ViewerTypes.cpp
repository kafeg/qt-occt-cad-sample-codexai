#include "ViewerTypes.h"
#include "OcctQmlViewer.h"

void registerViewerTypes()
{
  // Регистрация OcctQmlViewer как QML типа
  qmlRegisterType<OcctQmlViewer>("VibeCAD.Viewer", 1, 0, "OcctViewer");
}
