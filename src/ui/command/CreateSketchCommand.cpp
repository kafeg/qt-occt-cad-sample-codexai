#include "CreateSketchCommand.h"

#include <Document.h>
#include <PlaneFeature.h>
#include <Sketch.h>

#include <gp_Ax2.hxx>
#include <gp_Ax3.hxx>
#include <gp_Dir.hxx>
#include <cmath>

void CreateSketchCommand::execute(Document& doc)
{
  m_sketch.reset();
  if (m_plane.IsNull()) return;

  // Build an Ax2 from plane origin/normal; choose a stable X dir not colinear with normal
  const gp_Pnt o = m_plane->origin();
  const gp_Dir n = m_plane->normal();
  gp_Dir xdir(1.0, 0.0, 0.0);
  if (std::abs(n.Dot(xdir)) > 0.99) // near-colinear, use Y axis
    xdir = gp_Dir(0.0, 1.0, 0.0);
  gp_Ax2 ax2(o, n, xdir);

  auto sk = std::make_shared<Sketch>();
  sk->setPlane(ax2);
  sk->setPlaneId(m_plane->id());
  doc.addSketch(sk);
  m_sketch = std::move(sk);
}
