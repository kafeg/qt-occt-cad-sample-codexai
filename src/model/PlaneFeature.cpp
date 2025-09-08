#include "PlaneFeature.h"
#include <DocumentItem.h>

#include <BRepBuilderAPI_MakeFace.hxx>
#include <Geom_Plane.hxx>
#include <gp_Ax3.hxx>
#include <gp_Vec.hxx>
#include <AIS_Shape.hxx>

IMPLEMENT_STANDARD_RTTIEXT(PlaneFeature, Feature)

namespace {
const bool kPlaneFeatureReg = [](){
  DocumentItem::registerFactory(DocumentItem::Kind::PlaneFeature, [](){ return std::shared_ptr<DocumentItem>(new PlaneFeature()); });
  return true;
}();
}

gp_Pnt PlaneFeature::origin() const
{
  const double x = Feature::paramAsDouble(params(), Feature::ParamKey::Ox, 0.0);
  const double y = Feature::paramAsDouble(params(), Feature::ParamKey::Oy, 0.0);
  const double z = Feature::paramAsDouble(params(), Feature::ParamKey::Oz, 0.0);
  return gp_Pnt(x, y, z);
}

gp_Dir PlaneFeature::normal() const
{
  // Fallback to +Z if not set
  gp_Vec v(Feature::paramAsDouble(params(), Feature::ParamKey::Nx, 0.0),
           Feature::paramAsDouble(params(), Feature::ParamKey::Ny, 0.0),
           Feature::paramAsDouble(params(), Feature::ParamKey::Nz, 1.0));
  if (v.SquareMagnitude() <= gp::Resolution())
    v = gp_Vec(0.0, 0.0, 1.0);
  return gp_Dir(v);
}

double PlaneFeature::size() const
{
  return Feature::paramAsDouble(params(), Feature::ParamKey::Size, 100.0);
}

double PlaneFeature::transparency() const
{
  // Default semi-transparent as before (0.3)
  return Feature::paramAsDouble(params(), Feature::ParamKey::Transparency, 0.3);
}

void PlaneFeature::execute()
{
  const gp_Pnt o = origin();
  const gp_Dir n = normal();
  const double s = size();

  // Choose a stable X direction orthogonal to normal
  gp_Dir xdir(1.0, 0.0, 0.0);
  if (std::abs(n.Dot(xdir)) > 0.99) // near-colinear, switch to Y
    xdir = gp_Dir(0.0, 1.0, 0.0);
  // Build plane frame
  gp_Ax3 ax(o, n, xdir);
  Handle(Geom_Plane) plane = new Geom_Plane(ax);
  // Make rectangular face in plane UV: [-s, s] x [-s, s]
  m_shape = BRepBuilderAPI_MakeFace(Handle(Geom_Surface)(plane), -s, s, -s, s, 1.0e-7);
}

void PlaneFeature::applyStyle(const Handle(AIS_Shape)& ais) const
{
  if (ais.IsNull()) return;
  ais->SetColor(PlaneFeature::defaultColor());
  ais->SetTransparency((Standard_ShortReal)transparency());
}
