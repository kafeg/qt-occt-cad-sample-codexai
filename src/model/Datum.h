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
    , m_planeSize(150.0)       // default plane size used by gizmos
    , m_planeOffset(30.0)      // default offset from origin along axes for planes
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

private:
  gp_Pnt m_origin;
  gp_Dir m_dirX;
  gp_Dir m_dirY;
  gp_Dir m_dirZ;
  double m_axisLength;
  double m_planeSize;
  double m_planeOffset;
};

