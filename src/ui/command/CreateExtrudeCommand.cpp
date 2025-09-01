#include "CreateExtrudeCommand.h"

#include <Document.h>
#include <ExtrudeFeature.h>
#include <Sketch.h>

void CreateExtrudeCommand::execute(Document& doc)
{
  // Register sketch in the document registry and reference it by ID
  if (m_sketch) {
    doc.addSketch(m_sketch);
    // Also append the sketch into the document timeline so the history shows it
    Handle(Sketch) hs = new Sketch(m_sketch->id());
    hs->deserialize(m_sketch->serialize());
    doc.addItem(Handle(DocumentItem)(hs));
  }
  Handle(ExtrudeFeature) ef = new ExtrudeFeature();
  if (m_sketch) {
    ef->setSketchId(m_sketch->id());
  }
  ef->setDistance(m_distance);
  doc.addFeature(ef);
  doc.recompute();
}
