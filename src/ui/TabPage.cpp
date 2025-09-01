#include "TabPage.h"

#include <OcctQOpenGLWidgetViewer.h>

#include <Standard_WarningsDisable.hxx>
#include <QVBoxLayout>
#include <Standard_WarningsRestore.hxx>

#include <Document.h>
#include <Sketch.h>

TabPage::TabPage(QWidget* parent)
  : QWidget(parent)
{
  auto* lay = new QVBoxLayout(this);
  lay->setContentsMargins(0, 0, 0, 0);
  m_viewer = new OcctQOpenGLWidgetViewer(this);
  lay->addWidget(m_viewer);
  m_doc = std::make_unique<Document>();
}

TabPage::~TabPage() = default;
