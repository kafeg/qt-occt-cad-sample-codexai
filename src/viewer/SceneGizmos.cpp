// Simplified and robust gizmo manager implementation
#include "SceneGizmos.h"

#include <Datum.h>

#include <AIS_InteractiveContext.hxx>
#include <AIS_Trihedron.hxx>
#include <AIS_TrihedronSelectionMode.hxx>
#include <AIS_Shape.hxx>
#include <AIS_TexturedShape.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <Geom_Axis2Placement.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Line.hxx>
#include <Geom_Plane.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>
#include <Graphic3d_TransformPers.hxx>
#include <Graphic3d_ZLayerId.hxx>
#include <Image_PixMap.hxx>
#include <Prs3d_DatumAspect.hxx>
#include <Prs3d_LineAspect.hxx>
#include <Prs3d_ShadingAspect.hxx>
#include <Quantity_Color.hxx>
#include <gp_Ax2.hxx>
#include <gp_Ax3.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>

#include <QImage>
#include <cstring>

namespace
{
  // Colors and constants
  const Quantity_Color kColX(1.00, 0.20, 0.20, Quantity_TOC_sRGB);
  const Quantity_Color kColY(0.25, 0.90, 0.25, Quantity_TOC_sRGB);
  const Quantity_Color kColZ(0.25, 0.45, 1.00, Quantity_TOC_sRGB);
  const Quantity_Color kColPlane(0.95, 0.85, 0.70, Quantity_TOC_sRGB);
  constexpr Standard_Real kPlaneTransp = 0.3;
  constexpr Standard_Real kOriginMarkRadius = 10.0;

  inline gp_Pnt pMix(const gp_Pnt& o, const gp_Dir& a, double sa, const gp_Dir& b, double sb)
  {
    return gp_Pnt(o.X() + a.X() * sa + b.X() * sb,
                  o.Y() + a.Y() * sa + b.Y() * sb,
                  o.Z() + a.Z() * sa + b.Z() * sb);
  }

  static Handle(Image_PixMap) loadRgbaPixmap(const char* resPath)
  {
    QImage img(QString::fromUtf8(resPath));
    if (img.isNull()) return Handle(Image_PixMap)();
    QImage rgba = img.convertToFormat(QImage::Format_RGBA8888);
    Handle(Image_PixMap) out = new Image_PixMap();
    if (!out->InitZero(Image_Format_RGBA, rgba.width(), rgba.height(), rgba.bytesPerLine()))
      return Handle(Image_PixMap)();
    out->SetTopDown(true);
    const int h = rgba.height();
    for (int y = 0; y < h; ++y)
    {
      std::memcpy(out->ChangeRow(y), rgba.constScanLine(y), size_t(rgba.bytesPerLine()));
    }
    return out;
  }
}

void SceneGizmos::install(const Handle(AIS_InteractiveContext)& ctx,
                          const std::shared_ptr<Datum>&          datum,
                          Standard_Boolean                        topmostOverlay)
{
  if (ctx.IsNull() || !datum) return;

  const gp_Pnt ori = datum->origin();
  const gp_Dir dx  = datum->dirX();
  const gp_Dir dy  = datum->dirY();
  const gp_Dir dz  = datum->dirZ();
  const Standard_Real axLen     = datum->axisLength();
  const Standard_Real planeSize = datum->planeSize();
  const Standard_Real offset    = datum->planeOffset();

  const bool showTri = datum->showTrihedronAxes();
  const bool showOri = datum->showOriginPoint();
  const bool showXY  = datum->showPlaneXY();
  const bool showXZ  = datum->showPlaneXZ();
  const bool showYZ  = datum->showPlaneYZ();

  // Background axes are tied to trihedron visibility
  m_showBgAxes = showTri;

  // Trihedron (zoom persistent)
  if (showTri)
  {
    if (m_trihedron.IsNull())
    {
      Handle(Geom_Axis2Placement) a2 = new Geom_Axis2Placement(gp_Ax2(ori, dz, dx));
      m_trihedron = new AIS_Trihedron(a2);
      Handle(Prs3d_DatumAspect) dAsp = new Prs3d_DatumAspect();
      dAsp->SetAxisLength(axLen, axLen, axLen);
      Handle(Prs3d_Drawer) trD = new Prs3d_Drawer();
      trD->SetDatumAspect(dAsp);
      m_trihedron->SetAttributes(trD);
      m_trihedron->SetDatumPartColor(Prs3d_DatumParts_XAxis, kColX);
      m_trihedron->SetDatumPartColor(Prs3d_DatumParts_YAxis, kColY);
      m_trihedron->SetDatumPartColor(Prs3d_DatumParts_ZAxis, kColZ);
      m_trihedron->SetArrowColor(Prs3d_DatumParts_XAxis, kColX);
      m_trihedron->SetArrowColor(Prs3d_DatumParts_YAxis, kColY);
      m_trihedron->SetArrowColor(Prs3d_DatumParts_ZAxis, kColZ);
      m_trihedron->SetTransformPersistence(new Graphic3d_TransformPers(Graphic3d_TMF_ZoomPers, gp::Origin()));
      ctx->Display(m_trihedron, Standard_False);
      ctx->Activate(m_trihedron, AIS_TrihedronSelectionMode_Axes);
      ctx->Activate(m_trihedron, AIS_TrihedronSelectionMode_Origin);
    }
    else
    {
      // Update axis length and placement when Datum changed
      Handle(Geom_Axis2Placement) a2 = m_trihedron->Component();
      if (!a2.IsNull()) a2->SetAx2(gp_Ax2(ori, dz, dx));
      Handle(Prs3d_Drawer) trD = m_trihedron->Attributes();
      if (!trD.IsNull())
      {
        Handle(Prs3d_DatumAspect) dAsp = trD->DatumAspect();
        if (!dAsp.IsNull()) dAsp->SetAxisLength(axLen, axLen, axLen);
      }
      ctx->Redisplay(m_trihedron, Standard_False);
      ctx->Display(m_trihedron, Standard_False);
    }
    if (topmostOverlay && !m_trihedron.IsNull())
      ctx->SetZLayer(m_trihedron, Graphic3d_ZLayerId_Top);
  }
  else if (!m_trihedron.IsNull())
  {
    ctx->Erase(m_trihedron, Standard_False);
  }

  // Background axes (world-space lines, non-selectable)
  if (m_showBgAxes)
  {
    // Default extents (viewer can override via setAxisExtents)
    if (m_bgAxisX.IsNull() || m_bgAxisY.IsNull())
    {
      const Standard_Real halfX = 500.0;
      const Standard_Real halfY = 500.0;
      m_bgAxisX = new AIS_Shape(BRepBuilderAPI_MakeEdge(gp_Pnt(-halfX, 0.0, 0.0), gp_Pnt(halfX, 0.0, 0.0)));
      m_bgAxisY = new AIS_Shape(BRepBuilderAPI_MakeEdge(gp_Pnt(0.0, -halfY, 0.0), gp_Pnt(0.0, halfY, 0.0)));
      Handle(Prs3d_Drawer) dxAsp = new Prs3d_Drawer();
      dxAsp->SetLineAspect(new Prs3d_LineAspect(kColX, Aspect_TOL_SOLID, 2.0f));
      m_bgAxisX->SetAttributes(dxAsp);
      m_bgAxisX->SetDisplayMode(AIS_WireFrame);
      m_bgAxisX->SetAutoHilight(false);
      Handle(Prs3d_Drawer) dyAsp = new Prs3d_Drawer();
      dyAsp->SetLineAspect(new Prs3d_LineAspect(kColY, Aspect_TOL_SOLID, 2.0f));
      m_bgAxisY->SetAttributes(dyAsp);
      m_bgAxisY->SetDisplayMode(AIS_WireFrame);
      m_bgAxisY->SetAutoHilight(false);
      ctx->Display(m_bgAxisX, Standard_False);
      ctx->Display(m_bgAxisY, Standard_False);
      ctx->Deactivate(m_bgAxisX);
      ctx->Deactivate(m_bgAxisY);
    }
    else
    {
      ctx->Display(m_bgAxisX, Standard_False);
      ctx->Display(m_bgAxisY, Standard_False);
    }
  }
  else
  {
    if (!m_bgAxisX.IsNull()) ctx->Erase(m_bgAxisX, Standard_False);
    if (!m_bgAxisY.IsNull()) ctx->Erase(m_bgAxisY, Standard_False);
  }

  // Helper to build a rectangular plane aligned by two directions
  auto makeRectPlane = [&](const gp_Dir& a, const gp_Dir& b) -> Handle(AIS_Shape)
  {
    const gp_Pnt p1 = pMix(ori, a, offset,     b, offset);
    const gp_Pnt p2 = pMix(ori, a, planeSize,  b, offset);
    const gp_Pnt p3 = pMix(ori, a, planeSize,  b, planeSize);
    const gp_Pnt p4 = pMix(ori, a, offset,     b, planeSize);
    TopoDS_Wire w = BRepBuilderAPI_MakeWire(
        BRepBuilderAPI_MakeEdge(p1, p2),
        BRepBuilderAPI_MakeEdge(p2, p3),
        BRepBuilderAPI_MakeEdge(p3, p4),
        BRepBuilderAPI_MakeEdge(p4, p1));
    Handle(AIS_Shape) shp = new AIS_Shape(BRepBuilderAPI_MakeFace(w));
    shp->SetColor(kColPlane);
    shp->SetTransparency(kPlaneTransp);
    shp->SetDisplayMode(AIS_Shaded);
    shp->SetTransformPersistence(new Graphic3d_TransformPers(Graphic3d_TMF_ZoomPers, gp::Origin()));
    Handle(Prs3d_Drawer) dr = shp->Attributes();
    if (dr.IsNull()) { dr = new Prs3d_Drawer(); shp->SetAttributes(dr); }
    dr->SetFaceBoundaryDraw(Standard_False);
    return shp;
  };

  // YZ plane
  if (showYZ)
  {
    if (m_planeYZ.IsNull()) m_planeYZ = makeRectPlane(dy, dz);
    ctx->Display(m_planeYZ, Standard_False);
    if (topmostOverlay) ctx->SetZLayer(m_planeYZ, Graphic3d_ZLayerId_Top);
  }
  else if (!m_planeYZ.IsNull())
  {
    ctx->Erase(m_planeYZ, Standard_False);
  }

  // XZ plane
  if (showXZ)
  {
    if (m_planeXZ.IsNull()) m_planeXZ = makeRectPlane(dx, dz);
    ctx->Display(m_planeXZ, Standard_False);
    if (topmostOverlay) ctx->SetZLayer(m_planeXZ, Graphic3d_ZLayerId_Top);
  }
  else if (!m_planeXZ.IsNull())
  {
    ctx->Erase(m_planeXZ, Standard_False);
  }

  // XY plane
  if (showXY)
  {
    if (m_planeXY.IsNull()) m_planeXY = makeRectPlane(dx, dy);
    ctx->Display(m_planeXY, Standard_False);
    if (topmostOverlay) ctx->SetZLayer(m_planeXY, Graphic3d_ZLayerId_Top);
  }
  else if (!m_planeXY.IsNull())
  {
    ctx->Erase(m_planeXY, Standard_False);
  }

  // Origin mark (circle)
  if (showOri)
  {
    if (m_originMark.IsNull())
    {
      Handle(Geom_Circle) gC = new Geom_Circle(gp_Ax2(ori, dz, dx), kOriginMarkRadius);
      TopoDS_Edge e = BRepBuilderAPI_MakeEdge(gC);
      TopoDS_Wire w = BRepBuilderAPI_MakeWire(e);
      m_originMark = new AIS_Shape(BRepBuilderAPI_MakeFace(w));
      Handle(Prs3d_Drawer) dO = new Prs3d_Drawer();
      dO->SetColor(Quantity_Color(Quantity_NOC_WHITE));
      dO->SetFaceBoundaryDraw(Standard_True);
      dO->SetWireAspect(new Prs3d_LineAspect(Quantity_Color(Quantity_NOC_BLACK), Aspect_TOL_SOLID, 1.5f));
      m_originMark->SetAttributes(dO);
      m_originMark->SetDisplayMode(AIS_Shaded);
      m_originMark->SetTransformPersistence(new Graphic3d_TransformPers(Graphic3d_TMF_ZoomPers, gp::Origin()));
      m_originMark->SetAutoHilight(true);
    }
    ctx->Display(m_originMark, Standard_False);
    if (topmostOverlay) ctx->SetZLayer(m_originMark, Graphic3d_ZLayerId_Top);
  }
  else if (!m_originMark.IsNull())
  {
    ctx->Erase(m_originMark, Standard_False);
  }

  // Origin sprite (textured quad)
  if (showOri)
  {
    if (m_bgOriginSprite.IsNull())
    {
      const Standard_Real s = 48.0;
      Handle(Geom_Plane) plane = new Geom_Plane(gp_Ax3(ori, dz, dx));
      TopoDS_Face f = BRepBuilderAPI_MakeFace(Handle(Geom_Surface)(plane), -s * 0.5, s * 0.5, -s * 0.5, s * 0.5, 1.0e-7);
      m_bgOriginSprite = new AIS_TexturedShape(f);
      Handle(Image_PixMap) sprite = loadRgbaPixmap(":/images/circle.png");
      if (!sprite.IsNull())
      {
        m_bgOriginSprite->SetTexturePixMap(sprite);
        m_bgOriginSprite->SetTextureMapOn();
        m_bgOriginSprite->SetTextureRepeat(Standard_False, 1.0, 1.0);
        m_bgOriginSprite->DisableTextureModulate();
        Handle(Prs3d_Drawer) sprDr = new Prs3d_Drawer();
        Handle(Prs3d_ShadingAspect) sprShade = new Prs3d_ShadingAspect();
        Handle(Graphic3d_AspectFillArea3d) fillAsp = sprShade->Aspect();
        fillAsp->SetAlphaMode(Graphic3d_AlphaMode_Blend);
        fillAsp->SetPolygonOffsets(Aspect_POM_Fill, 1.0f, 1.0f);
        sprShade->SetTransparency(0.9f);
        sprDr->SetShadingAspect(sprShade);
        m_bgOriginSprite->SetAttributes(sprDr);
        m_bgOriginSprite->SetDisplayMode(3);
        m_bgOriginSprite->SetAutoHilight(false);
        m_bgOriginSprite->SetTransparency(0.9f);
        m_bgOriginSprite->SetTransformPersistence(new Graphic3d_TransformPers(Graphic3d_TMF_ZoomPers, ori));
      }
    }
    ctx->Display(m_bgOriginSprite, Standard_False);
    ctx->SetZLayer(m_bgOriginSprite, Graphic3d_ZLayerId_Default);
    ctx->Deactivate(m_bgOriginSprite);
  }
  else if (!m_bgOriginSprite.IsNull())
  {
    ctx->Erase(m_bgOriginSprite, Standard_False);
  }
}

void SceneGizmos::reinstall(const Handle(AIS_InteractiveContext)& ctx)
{
  if (ctx.IsNull()) return;
  if (!m_bgAxisX.IsNull()) ctx->Display(m_bgAxisX, Standard_False);
  if (!m_bgAxisY.IsNull()) ctx->Display(m_bgAxisY, Standard_False);
  if (!m_trihedron.IsNull()) ctx->Display(m_trihedron, Standard_False);
  if (!m_planeYZ.IsNull()) ctx->Display(m_planeYZ, Standard_False);
  if (!m_planeXZ.IsNull()) ctx->Display(m_planeXZ, Standard_False);
  if (!m_planeXY.IsNull()) ctx->Display(m_planeXY, Standard_False);
  if (!m_originMark.IsNull()) ctx->Display(m_originMark, Standard_False);
  if (!m_bgOriginSprite.IsNull()) ctx->Display(m_bgOriginSprite, Standard_False);
}

void SceneGizmos::erase(const Handle(AIS_InteractiveContext)& ctx)
{
  if (ctx.IsNull()) return;
  if (!m_bgAxisX.IsNull()) ctx->Erase(m_bgAxisX, Standard_False);
  if (!m_bgAxisY.IsNull()) ctx->Erase(m_bgAxisY, Standard_False);
  if (!m_trihedron.IsNull()) ctx->Erase(m_trihedron, Standard_False);
  if (!m_planeYZ.IsNull()) ctx->Erase(m_planeYZ, Standard_False);
  if (!m_planeXZ.IsNull()) ctx->Erase(m_planeXZ, Standard_False);
  if (!m_planeXY.IsNull()) ctx->Erase(m_planeXY, Standard_False);
  if (!m_originMark.IsNull()) ctx->Erase(m_originMark, Standard_False);
  if (!m_bgOriginSprite.IsNull()) ctx->Erase(m_bgOriginSprite, Standard_False);
}

void SceneGizmos::setAxisExtents(const Handle(AIS_InteractiveContext)& ctx, Standard_Real halfX, Standard_Real halfY)
{
  if (ctx.IsNull()) return;
  if (!m_showBgAxes)
  {
    if (!m_bgAxisX.IsNull()) { ctx->Erase(m_bgAxisX, Standard_False); }
    if (!m_bgAxisY.IsNull()) { ctx->Erase(m_bgAxisY, Standard_False); }
    return;
  }
  if (!m_bgAxisX.IsNull()) ctx->Erase(m_bgAxisX, Standard_False);
  if (!m_bgAxisY.IsNull()) ctx->Erase(m_bgAxisY, Standard_False);
  m_bgAxisX = new AIS_Shape(BRepBuilderAPI_MakeEdge(gp_Pnt(-halfX, 0.0, 0.0), gp_Pnt(halfX, 0.0, 0.0)));
  m_bgAxisY = new AIS_Shape(BRepBuilderAPI_MakeEdge(gp_Pnt(0.0, -halfY, 0.0), gp_Pnt(0.0, halfY, 0.0)));
  Handle(Prs3d_Drawer) dxAsp = new Prs3d_Drawer(); dxAsp->SetLineAspect(new Prs3d_LineAspect(kColX, Aspect_TOL_SOLID, 2.0f));
  m_bgAxisX->SetAttributes(dxAsp); m_bgAxisX->SetDisplayMode(AIS_WireFrame); m_bgAxisX->SetAutoHilight(false);
  Handle(Prs3d_Drawer) dyAsp = new Prs3d_Drawer(); dyAsp->SetLineAspect(new Prs3d_LineAspect(kColY, Aspect_TOL_SOLID, 2.0f));
  m_bgAxisY->SetAttributes(dyAsp); m_bgAxisY->SetDisplayMode(AIS_WireFrame); m_bgAxisY->SetAutoHilight(false);
  ctx->Display(m_bgAxisX, Standard_False);
  ctx->Display(m_bgAxisY, Standard_False);
  ctx->Deactivate(m_bgAxisX);
  ctx->Deactivate(m_bgAxisY);
  ctx->SetZLayer(m_bgAxisX, Graphic3d_ZLayerId_Default);
  ctx->SetZLayer(m_bgAxisY, Graphic3d_ZLayerId_Default);
}
