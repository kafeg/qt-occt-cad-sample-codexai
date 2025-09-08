#include <QtQml/qqml.h>
#include "OcctQmlViewer.h"

static void registerOcctViewerQmlTypes()
{
  qmlRegisterType<OcctQmlViewer>("Occt.Viewer", 1, 0, "OcctViewer");
}

Q_COREAPP_STARTUP_FUNCTION(registerOcctViewerQmlTypes)

