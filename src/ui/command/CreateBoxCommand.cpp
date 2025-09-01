#include "CreateBoxCommand.h"

#include <Document.h>
#include <BoxFeature.h>

void CreateBoxCommand::execute(Document& doc)
{
  Handle(BoxFeature) bf = new BoxFeature();
  bf->setSize(m_dx, m_dy, m_dz);
  doc.addFeature(bf);
  doc.recompute();
}

