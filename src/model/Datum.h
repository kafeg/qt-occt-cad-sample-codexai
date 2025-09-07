#pragma once

#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
#include <memory>

// Datum: global scene reference for a Document
// - Origin point (world origin)
// - Canonical axes directions (positive X/Y/Z)
// - Visual extents for axes and helper planes
class Datum {
public:
  Datum()
    : m_origin(0.0, 0.0, 0.0)
    , m_dirX(1.0, 0.0, 0.0)
    , m_dirY(0.0, 1.0, 0.0)
    , m_dirZ(0.0, 0.0, 1.0)
    , m_axisLength(300.0)      // analogous to previous trihedron axis length
    , m_planeSize(40.0)       // default plane size used by gizmos
    , m_planeOffset(10.0)      // default offset from origin along axes for planes
    , m_showTrihedronAxes(true)
    , m_showOriginPoint(true)
    , m_showPlaneXY(true)
    , m_showPlaneXZ(true)
    , m_showPlaneYZ(true)
    , m_showTriX(true)
    , m_showTriY(true)
    , m_showTriZ(true)
  {}

  // Origin
  const gp_Pnt& origin() const { return m_origin; }
  void setOrigin(const gp_Pnt& p) { m_origin = p; }

  // Axes directions (unit vectors)
  const gp_Dir& dirX() const { return m_dirX; }
  const gp_Dir& dirY() const { return m_dirY; }
  const gp_Dir& dirZ() const { return m_dirZ; }
  void setDirs(const gp_Dir& dx, const gp_Dir& dy, const gp_Dir& dz) { m_dirX = dx; m_dirY = dy; m_dirZ = dz; }

  // Visual parameters
  double axisLength()   const { return m_axisLength; }
  double planeSize()    const { return m_planeSize; }
  double planeOffset()  const { return m_planeOffset; }
  void setAxisLength(double v)  { m_axisLength = v; }
  void setPlaneSize(double v)   { m_planeSize  = v; }
  void setPlaneOffset(double v) { m_planeOffset = v; }

  // Visibility toggles (to be controlled from Document/UI)
  bool showTrihedronAxes() const { return m_showTrihedronAxes; }
  bool showOriginPoint()   const { return m_showOriginPoint; }
  bool showPlaneXY()       const { return m_showPlaneXY; }
  bool showPlaneXZ()       const { return m_showPlaneXZ; }
  bool showPlaneYZ()       const { return m_showPlaneYZ; }

  void setShowTrihedronAxes(bool v) { m_showTrihedronAxes = v; }
  void setShowOriginPoint(bool v)   { m_showOriginPoint   = v; }
  void setShowPlaneXY(bool v)       { m_showPlaneXY       = v; }
  void setShowPlaneXZ(bool v)       { m_showPlaneXZ       = v; }
  void setShowPlaneYZ(bool v)       { m_showPlaneYZ       = v; }

  // Per-axis trihedron visibility
  bool showTrihedronAxisX() const { return m_showTriX; }
  bool showTrihedronAxisY() const { return m_showTriY; }
  bool showTrihedronAxisZ() const { return m_showTriZ; }
  void setShowTrihedronAxisX(bool v) { m_showTriX = v; }
  void setShowTrihedronAxisY(bool v) { m_showTriY = v; }
  void setShowTrihedronAxisZ(bool v) { m_showTriZ = v; }

private:
  gp_Pnt m_origin;
  gp_Dir m_dirX;
  gp_Dir m_dirY;
  gp_Dir m_dirZ;
  double m_axisLength;
  double m_planeSize;
  double m_planeOffset;

  bool   m_showTrihedronAxes;
  bool   m_showOriginPoint;
  bool   m_showPlaneXY;
  bool   m_showPlaneXZ;
  bool   m_showPlaneYZ;
  bool   m_showTriX;
  bool   m_showTriY;
  bool   m_showTriZ;
};
