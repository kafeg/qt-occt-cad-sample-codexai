#include "CreateBoxCommand.h"

#include <Document.h>
#include <BoxFeature.h>

void CreateBoxCommand::execute(Document& doc)
{
  Handle(BoxFeature) bf = new BoxFeature(myDx, myDy, myDz);
  doc.addFeature(bf);
  doc.recompute();
}
