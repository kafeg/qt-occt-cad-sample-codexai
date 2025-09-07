#include "DocumentInitializer.h"

#include "Document.h"
#include "PlaneFeature.h"
#include "PointFeature.h"
#include "Datum.h"
#include "AxeFeature.h"

#include <gp_Vec.hxx>

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
  const Standard_Real offset    = d->planeOffset();

  // Match previous UI-created geometry extents
  const Standard_Real half = 0.5 * (planeSize - offset);
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
    pXY->setTransparency(0.3);
    pXY->setName(TCollection_AsciiString("Plane XY"));
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
    pXY->setTransparency(0.3);
    pXY->setName(TCollection_AsciiString("Plane XY"));
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
    pXZ->setTransparency(0.3);
    pXZ->setName(TCollection_AsciiString("Plane XZ"));
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
    pXZ->setTransparency(0.3);
    pXZ->setName(TCollection_AsciiString("Plane XZ"));
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
    pYZ->setTransparency(0.3);
    pYZ->setName(TCollection_AsciiString("Plane YZ"));
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
    pYZ->setTransparency(0.3);
    pYZ->setName(TCollection_AsciiString("Plane YZ"));
    pYZ->setSuppressed(true);
    doc.addPlane(pYZ);
  }

  // Optional origin point
  if (d->showOriginPoint())
  {
    Handle(PointFeature) pt = new PointFeature();
    pt->setOrigin(d->origin());
    pt->setRadius(10.0);
    pt->setFixedGeometry(true);
    pt->setName(TCollection_AsciiString("Origin"));
    doc.addFeature(pt);
  }

  // Axes (X, Y, Z) as fixed geometry edges, visibility bound to Datum toggles
  {
    const double axLen = d->axisLength();
    // X axis
    Handle(AxeFeature) axX = new AxeFeature();
    axX->setOrigin(ori);
    axX->setDirection(dx);
    axX->setLength(axLen);
    axX->setFixedGeometry(true);
    axX->setName(TCollection_AsciiString("Axis X"));
    axX->setSuppressed(!d->showTrihedronAxisX());
    doc.addFeature(axX);

    // Y axis
    Handle(AxeFeature) axY = new AxeFeature();
    axY->setOrigin(ori);
    axY->setDirection(dy);
    axY->setLength(axLen);
    axY->setFixedGeometry(true);
    axY->setName(TCollection_AsciiString("Axis Y"));
    axY->setSuppressed(!d->showTrihedronAxisY());
    doc.addFeature(axY);

    // Z axis
    Handle(AxeFeature) axZ = new AxeFeature();
    axZ->setOrigin(ori);
    axZ->setDirection(dz);
    axZ->setLength(axLen);
    axZ->setFixedGeometry(true);
    axZ->setName(TCollection_AsciiString("Axis Z"));
    axZ->setSuppressed(!d->showTrihedronAxisZ());
    doc.addFeature(axZ);
  }

  // Recompute to ensure shapes are built for immediate use
  doc.recompute();
}

} // namespace DocumentInitializer
