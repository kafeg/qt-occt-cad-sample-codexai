#include "CreateCylinderCommand.h"

#include <Document.h>
#include <CylinderFeature.h>

void CreateCylinderCommand::execute(Document& doc)
{
  Handle(CylinderFeature) cf = new CylinderFeature();
  cf->set(myRadius, myHeight);
  doc.addFeature(cf);
  doc.recompute();
}
