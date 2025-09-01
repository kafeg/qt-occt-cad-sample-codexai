#include "CreateCylinderCommand.h"

#include <Document.h>
#include <CylinderFeature.h>

void CreateCylinderCommand::execute(Document& doc)
{
  Handle(CylinderFeature) cf = new CylinderFeature();
  cf->set(m_radius, m_height);
  doc.addFeature(cf);
  doc.recompute();
}

