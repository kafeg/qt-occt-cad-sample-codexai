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

  // Trihedron (zoom persistent) — disabled per request (hidden)
  if (false && showTri)
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
      // Ensure only per-part selection modes are active (no entire-object selection)
      ctx->Deactivate(m_trihedron);
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
      // Keep only axes/origin selection modes enabled
      ctx->Deactivate(m_trihedron);
      ctx->Activate(m_trihedron, AIS_TrihedronSelectionMode_Axes);
      ctx->Activate(m_trihedron, AIS_TrihedronSelectionMode_Origin);
    }
    if (topmostOverlay && !m_trihedron.IsNull())
      ctx->SetZLayer(m_trihedron, Graphic3d_ZLayerId_Top);
  }
  else if (!m_trihedron.IsNull())
  {
    ctx->Erase(m_trihedron, Standard_False);
  }

  // Background axes — disabled per request (hidden)
  if (!m_bgAxisX.IsNull()) ctx->Erase(m_bgAxisX, Standard_False);
  if (!m_bgAxisY.IsNull()) ctx->Erase(m_bgAxisY, Standard_False);

  // Helper to build a rectangular plane aligned by two directions
  // No overlay planes; planes should be explicit document items

  // Origin mark is now a PointFeature in the Document; no overlay here.

  // Origin sprite (textured quad) - always visible, global scene at world origin
  if (m_bgOriginSprite.IsNull())
  {
    const Standard_Real s = 48.0;
    Handle(Geom_Plane) plane = new Geom_Plane(gp_Ax3(gp::Origin(), gp::DZ(), gp::DX()));
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
      m_bgOriginSprite->SetTransformPersistence(new Graphic3d_TransformPers(Graphic3d_TMF_ZoomPers, gp::Origin()));
    }
  }
  ctx->Display(m_bgOriginSprite, Standard_False);
  ctx->SetZLayer(m_bgOriginSprite, Graphic3d_ZLayerId_Default);
  ctx->Deactivate(m_bgOriginSprite);
}

void SceneGizmos::reinstall(const Handle(AIS_InteractiveContext)& ctx)
{
  if (ctx.IsNull()) return;
  // Keep trihedron and background axes hidden
  if (!m_bgAxisX.IsNull()) ctx->Erase(m_bgAxisX, Standard_False);
  if (!m_bgAxisY.IsNull()) ctx->Erase(m_bgAxisY, Standard_False);
  if (!m_trihedron.IsNull()) ctx->Erase(m_trihedron, Standard_False);
  // no overlay planes to reinstall
  if (!m_bgOriginSprite.IsNull()) ctx->Display(m_bgOriginSprite, Standard_False);
}

void SceneGizmos::erase(const Handle(AIS_InteractiveContext)& ctx)
{
  if (ctx.IsNull()) return;
  if (!m_bgAxisX.IsNull()) ctx->Erase(m_bgAxisX, Standard_False);
  if (!m_bgAxisY.IsNull()) ctx->Erase(m_bgAxisY, Standard_False);
  if (!m_trihedron.IsNull()) ctx->Erase(m_trihedron, Standard_False);
  // no overlay planes to erase
  if (!m_bgOriginSprite.IsNull()) ctx->Erase(m_bgOriginSprite, Standard_False);
}

void SceneGizmos::setAxisExtents(const Handle(AIS_InteractiveContext)& ctx, Standard_Real halfX, Standard_Real halfY)
{
  if (ctx.IsNull()) return;
  // Background axes are hidden; ensure they stay erased and do not recreate
  if (!m_bgAxisX.IsNull()) ctx->Erase(m_bgAxisX, Standard_False);
  if (!m_bgAxisY.IsNull()) ctx->Erase(m_bgAxisY, Standard_False);
  m_bgAxisX.Nullify();
  m_bgAxisY.Nullify();
}

void SceneGizmos::setTrihedronAxesVisibility(const Handle(AIS_InteractiveContext)& ctx,
                                             bool showX, bool showY, bool showZ)
{
  if (ctx.IsNull() || m_trihedron.IsNull()) return;
  Handle(Prs3d_Drawer) trD = m_trihedron->Attributes();
  if (trD.IsNull()) trD = new Prs3d_Drawer();
  Handle(Prs3d_DatumAspect) dAsp = trD->DatumAspect();
  if (dAsp.IsNull())
  {
    dAsp = new Prs3d_DatumAspect();
    trD->SetDatumAspect(dAsp);
    m_trihedron->SetAttributes(trD);
  }
  Standard_Integer mask = 0;
  if (showX) mask |= Prs3d_DatumAxes_XAxis;
  if (showY) mask |= Prs3d_DatumAxes_YAxis;
  if (showZ) mask |= Prs3d_DatumAxes_ZAxis;
  if (mask == 0)
  {
    // Keep object but hide all axes; Redisplay to update presentation
    dAsp->SetDrawDatumAxes(Prs3d_DatumAxes(0));
  }
  else
  {
    dAsp->SetDrawDatumAxes(Prs3d_DatumAxes(mask));
  }
  ctx->Redisplay(m_trihedron, Standard_False);
  ctx->Display(m_trihedron, Standard_False);
}
