#include "TabPage.h"

#include <OcctQOpenGLWidgetViewer.h>

#include <Standard_WarningsDisable.hxx>
#include <QVBoxLayout>
#include <Standard_WarningsRestore.hxx>

#include <Document.h>

TabPage::TabPage(QWidget* parent)
  : QWidget(parent)
{
  auto* lay = new QVBoxLayout(this);
  lay->setContentsMargins(0, 0, 0, 0);
  myViewer = new OcctQOpenGLWidgetViewer(this);
  lay->addWidget(myViewer);
  myDoc = std::make_unique<Document>();
}

TabPage::~TabPage() = default;

