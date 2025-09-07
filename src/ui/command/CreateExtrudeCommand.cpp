#include "CreateExtrudeCommand.h"

#include <Document.h>
#include <ExtrudeFeature.h>
#include <Sketch.h>
#include <gp_Ax2.hxx>
#include <gp_Dir.hxx>

void CreateExtrudeCommand::execute(Document& doc)
{
  // Register sketch in the document registry and reference it by ID
  if (!m_sketch) {
    // Create a default sketch on XY plane at origin
    m_sketch = std::make_shared<Sketch>();
    m_sketch->setPlane(gp_Ax2(gp_Pnt(0,0,0), gp::DZ(), gp::DX()));
    // Optional: keep planeId unset here; binding can be added later if needed
  }
  doc.addSketch(m_sketch);
  Handle(ExtrudeFeature) ef = new ExtrudeFeature();
  if (m_sketch) {
    ef->setSketchId(m_sketch->id());
  }
  ef->setDistance(m_distance);
  doc.addFeature(ef);
  doc.recompute();
}
