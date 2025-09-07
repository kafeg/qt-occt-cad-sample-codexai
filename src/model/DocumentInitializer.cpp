#include "DocumentInitializer.h"

#include "Document.h"
#include "PlaneFeature.h"
#include "PointFeature.h"
#include "Datum.h"
#include "AxeFeature.h"

#include <gp_Vec.hxx>

namespace {
// Centralized initializer tuning to avoid magic numbers in code
static constexpr Standard_Real K_PLANE_TRANSPARENCY         = 0.3;  // default plane transparency
static constexpr Standard_Real K_POINT_RADIUS               = 10.0; // default origin sphere radius
static constexpr Standard_Real K_AXIS_ORIGIN_OFFSET         = 30.0; // shift axes start from world origin
static constexpr Standard_Real K_PLANE_EDGE_OFFSET_DEFAULT  = 30.0; // fallback edge offset for planes if Datum not set

static inline gp_Pnt offsetAlong(const gp_Pnt& p, const gp_Dir& d, Standard_Real dist)
{
  gp_Vec v(d.XYZ()); v.Multiply(dist);
  return p.Translated(v);
}
}

namespace DocumentInitializer {

void initialize(Document& doc)
{
  auto d = doc.datum();
  if (!d) return;

  const gp_Pnt ori = d->origin();
  const gp_Dir dx  = d->dirX();
  const gp_Dir dy  = d->dirY();
  const gp_Dir dz  = d->dirZ();
  const Standard_Real planeSize = d->planeSize();
  // Prefer value supplied by Datum, but keep a clear named default
  const Standard_Real offset    = (d->planeOffset() > 0.0 ? d->planeOffset() : K_PLANE_EDGE_OFFSET_DEFAULT);

  // Match previous UI-created geometry extents
  // Compute a plane centered at distance `centerOff` along two axes so that
  // the nearest edge to origin is exactly at `offset` from origin:
  // nearest = (planeSize + offset)/2 - (planeSize - offset)/2 = offset
  const Standard_Real half      = 0.5 * (planeSize - offset);
  const Standard_Real centerOff = 0.5 * (planeSize + offset);

  auto mkPt = [&](const gp_Dir& a, const gp_Dir& b) {
    gp_Vec va(a.XYZ()); va.Multiply(centerOff);
    gp_Vec vb(b.XYZ()); vb.Multiply(centerOff);
    return ori.Translated(va + vb);
  };

  // XY
  if (d->showPlaneXY())
  {
    Handle(PlaneFeature) pXY = new PlaneFeature();
    pXY->setOrigin(mkPt(dx, dy));
    pXY->setNormal(dz);
    pXY->setSize(half);
    pXY->setFixedGeometry(true);
    pXY->setTransparency(K_PLANE_TRANSPARENCY);
    pXY->setName(TCollection_AsciiString("Plane XY"));
    pXY->setDatumRelated(true);
    pXY->setSuppressed(!d->showPlaneXY());
    doc.addPlane(pXY);
  }
  else
  {
    Handle(PlaneFeature) pXY = new PlaneFeature();
    pXY->setOrigin(mkPt(dx, dy));
    pXY->setNormal(dz);
    pXY->setSize(half);
    pXY->setFixedGeometry(true);
    pXY->setTransparency(K_PLANE_TRANSPARENCY);
    pXY->setName(TCollection_AsciiString("Plane XY"));
    pXY->setDatumRelated(true);
    pXY->setSuppressed(true);
    doc.addPlane(pXY);
  }

  // XZ
  if (d->showPlaneXZ())
  {
    Handle(PlaneFeature) pXZ = new PlaneFeature();
    pXZ->setOrigin(mkPt(dx, dz));
    pXZ->setNormal(dy);
    pXZ->setSize(half);
    pXZ->setFixedGeometry(true);
    pXZ->setTransparency(K_PLANE_TRANSPARENCY);
    pXZ->setName(TCollection_AsciiString("Plane XZ"));
    pXZ->setDatumRelated(true);
    pXZ->setSuppressed(!d->showPlaneXZ());
    doc.addPlane(pXZ);
  }
  else
  {
    Handle(PlaneFeature) pXZ = new PlaneFeature();
    pXZ->setOrigin(mkPt(dx, dz));
    pXZ->setNormal(dy);
    pXZ->setSize(half);
    pXZ->setFixedGeometry(true);
    pXZ->setTransparency(K_PLANE_TRANSPARENCY);
    pXZ->setName(TCollection_AsciiString("Plane XZ"));
    pXZ->setDatumRelated(true);
    pXZ->setSuppressed(true);
    doc.addPlane(pXZ);
  }

  // YZ
  if (d->showPlaneYZ())
  {
    Handle(PlaneFeature) pYZ = new PlaneFeature();
    pYZ->setOrigin(mkPt(dy, dz));
    pYZ->setNormal(dx);
    pYZ->setSize(half);
    pYZ->setFixedGeometry(true);
    pYZ->setTransparency(K_PLANE_TRANSPARENCY);
    pYZ->setName(TCollection_AsciiString("Plane YZ"));
    pYZ->setDatumRelated(true);
    pYZ->setSuppressed(!d->showPlaneYZ());
    doc.addPlane(pYZ);
  }
  else
  {
    Handle(PlaneFeature) pYZ = new PlaneFeature();
    pYZ->setOrigin(mkPt(dy, dz));
    pYZ->setNormal(dx);
    pYZ->setSize(half);
    pYZ->setFixedGeometry(true);
    pYZ->setTransparency(K_PLANE_TRANSPARENCY);
    pYZ->setName(TCollection_AsciiString("Plane YZ"));
    pYZ->setDatumRelated(true);
    pYZ->setSuppressed(true);
    doc.addPlane(pYZ);
  }

  // Optional origin point
  if (d->showOriginPoint())
  {
    Handle(PointFeature) pt = new PointFeature();
    pt->setOrigin(d->origin());
    pt->setRadius(K_POINT_RADIUS);
    pt->setFixedGeometry(true);
    pt->setName(TCollection_AsciiString("Origin"));
    pt->setDatumRelated(true);
    doc.addFeature(pt);
  }

  // Axes (X, Y, Z) as fixed geometry edges, visibility bound to Datum toggles
  {
    const double axLen = d->axisLength();
    // X axis
    Handle(AxeFeature) axX = new AxeFeature();
    axX->setOrigin(offsetAlong(ori, dx, K_AXIS_ORIGIN_OFFSET));
    axX->setDirection(dx);
    axX->setLength(axLen);
    axX->setFixedGeometry(true);
    axX->setName(TCollection_AsciiString("Axis X"));
    axX->setDatumRelated(true);
    axX->setSuppressed(!d->showTrihedronAxisX());
    doc.addFeature(axX);

    // Y axis
    Handle(AxeFeature) axY = new AxeFeature();
    axY->setOrigin(offsetAlong(ori, dy, K_AXIS_ORIGIN_OFFSET));
    axY->setDirection(dy);
    axY->setLength(axLen);
    axY->setFixedGeometry(true);
    axY->setName(TCollection_AsciiString("Axis Y"));
    axY->setDatumRelated(true);
    axY->setSuppressed(!d->showTrihedronAxisY());
    doc.addFeature(axY);

    // Z axis
    Handle(AxeFeature) axZ = new AxeFeature();
    axZ->setOrigin(offsetAlong(ori, dz, K_AXIS_ORIGIN_OFFSET));
    axZ->setDirection(dz);
    axZ->setLength(axLen);
    axZ->setFixedGeometry(true);
    axZ->setName(TCollection_AsciiString("Axis Z"));
    axZ->setDatumRelated(true);
    axZ->setSuppressed(!d->showTrihedronAxisZ());
    doc.addFeature(axZ);
  }

  // Recompute to ensure shapes are built for immediate use
  doc.recompute();
}

} // namespace DocumentInitializer
