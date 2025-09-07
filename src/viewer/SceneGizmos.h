// Axes + Trihedron overlay manager
#ifndef _SceneGizmos_HeaderFile
#define _SceneGizmos_HeaderFile

#include <AIS_InteractiveContext.hxx>
#include <AIS_Line.hxx>
#include <AIS_Plane.hxx>
#include <AIS_Shape.hxx>
#include <Geom_Plane.hxx>
#include <AIS_Trihedron.hxx>
#include <AIS_TrihedronSelectionMode.hxx>
#include <Geom_Axis2Placement.hxx>
#include <Prs3d_DatumAspect.hxx>
#include <Prs3d_LineAspect.hxx>
#include <Graphic3d_ZLayerId.hxx>
#include <Graphic3d_TransformPers.hxx>
#include <Quantity_Color.hxx>
#include <Geom_Line.hxx>
#include <Geom_CartesianPoint.hxx>
#include <Image_AlienPixMap.hxx>
#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
#include <gp.hxx>
#include <gp_Ax3.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <Geom_Circle.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>
#include <BRep_Builder.hxx>
#include <TopoDS_Compound.hxx>
#include <Aspect_TypeOfFacingModel.hxx>
#include <Graphic3d_MaterialAspect.hxx>
#include <Graphic3d_NameOfMaterial.hxx>
#include <Aspect_InteriorStyle.hxx>
#include <Graphic3d_Texture2D.hxx>
#include <Prs3d_ShadingAspect.hxx>
#include <AIS_TexturedShape.hxx>
#include <TCollection_AsciiString.hxx>
#include <Image_PixMap.hxx>
#include <QImage>
#include <QResource>
#include <cstring>
#include <memory>

#include <Datum.h>

class SceneGizmos
{
public:
  SceneGizmos() = default;

  void install(const Handle(AIS_InteractiveContext)& ctx,
               const std::shared_ptr<Datum>&          datum,
               Standard_Boolean                        topmostOverlay = Standard_True)
  {
    if (ctx.IsNull()) return;
    if (!datum) return;

    // Colors
    const Quantity_Color colX(1.00, 0.20, 0.20, Quantity_TOC_sRGB);
    const Quantity_Color colY(0.25, 0.90, 0.25, Quantity_TOC_sRGB);
    const Quantity_Color colZ(0.25, 0.45, 1.00, Quantity_TOC_sRGB);
    const Quantity_Color colPlane(0.95, 0.85, 0.7, Quantity_TOC_sRGB); // Soft peachy beige
    const Standard_Real planeSize = datum->planeSize();
    const Standard_Real offset = datum->planeOffset();
    const Standard_Real transparency = 0.3f; // semi-transparent planes
    // Visibility toggles from Document's Datum
    const bool showTri = datum->showTrihedronAxes();
    const bool showOri = datum->showOriginPoint();
    const bool showXY  = datum->showPlaneXY();
    const bool showXZ  = datum->showPlaneXZ();
    const bool showYZ  = datum->showPlaneYZ();

    // Create background long X/Y axes (world-space, non-selectable, thin)
    const Standard_Real halfX = 500.0;
    const Standard_Real halfY = 500.0;
    m_bgAxisX = new AIS_Shape(BRepBuilderAPI_MakeEdge(gp_Pnt(-halfX, 0.0, 0.0), gp_Pnt(halfX, 0.0, 0.0)));
    m_bgAxisY = new AIS_Shape(BRepBuilderAPI_MakeEdge(gp_Pnt(0.0, -halfY, 0.0), gp_Pnt(0.0, halfY, 0.0)));
    Handle(Prs3d_Drawer) bgDx = m_bgAxisX->Attributes(); if (bgDx.IsNull()) bgDx = new Prs3d_Drawer();
    bgDx->SetLineAspect(new Prs3d_LineAspect(colX, Aspect_TOL_SOLID, 2.0f));
    m_bgAxisX->SetAttributes(bgDx);
    m_bgAxisX->SetColor(colX);
    m_bgAxisX->SetDisplayMode(AIS_WireFrame);
    m_bgAxisX->SetAutoHilight(false);
    Handle(Prs3d_Drawer) bgDy = m_bgAxisY->Attributes(); if (bgDy.IsNull()) bgDy = new Prs3d_Drawer();
    bgDy->SetLineAspect(new Prs3d_LineAspect(colY, Aspect_TOL_SOLID, 2.0f));
    m_bgAxisY->SetAttributes(bgDy);
    m_bgAxisY->SetColor(colY);
    m_bgAxisY->SetDisplayMode(AIS_WireFrame);
    m_bgAxisY->SetAutoHilight(false);
    ctx->Display(m_bgAxisX, Standard_False);
    ctx->Display(m_bgAxisY, Standard_False);
    ctx->Deactivate(m_bgAxisX);
    ctx->Deactivate(m_bgAxisY);

    // Create trihedron from Datum (fixed on-screen size)
    const gp_Pnt ori = datum->origin();
    const gp_Dir dx  = datum->dirX();
    const gp_Dir dy  = datum->dirY();
    const gp_Dir dz  = datum->dirZ();
    const Standard_Real axLen = datum->axisLength();
    if (showTri)
    {
      Handle(Geom_Axis2Placement) a2 = new Geom_Axis2Placement(gp_Ax2(ori, dz, dx));
      m_trihedron = new AIS_Trihedron(a2);
      {
        Handle(Prs3d_Drawer) trD = m_trihedron->Attributes(); if (trD.IsNull()) trD = new Prs3d_Drawer();
        Handle(Prs3d_DatumAspect) dAsp = trD->DatumAspect(); if (dAsp.IsNull()) dAsp = new Prs3d_DatumAspect();
        dAsp->SetAxisLength(axLen, axLen, axLen);
        trD->SetDatumAspect(dAsp);
        m_trihedron->SetAttributes(trD);
        m_trihedron->SetDatumPartColor(Prs3d_DatumParts_XAxis, colX);
        m_trihedron->SetDatumPartColor(Prs3d_DatumParts_YAxis, colY);
        m_trihedron->SetDatumPartColor(Prs3d_DatumParts_ZAxis, colZ);
        m_trihedron->SetArrowColor(Prs3d_DatumParts_XAxis, colX);
        m_trihedron->SetArrowColor(Prs3d_DatumParts_YAxis, colY);
        m_trihedron->SetArrowColor(Prs3d_DatumParts_ZAxis, colZ);
        m_trihedron->SetTransformPersistence(new Graphic3d_TransformPers(Graphic3d_TMF_ZoomPers, gp::Origin()));
      }
      ctx->Display(m_trihedron, Standard_False);
      // Enable per-axis picking on trihedron
      ctx->Activate(m_trihedron, AIS_TrihedronSelectionMode_Axes);
      // Optionally keep origin picking as well
      ctx->Activate(m_trihedron, AIS_TrihedronSelectionMode_Origin);
      // keep selectable
    }

    // Create YZ plane using Datum directions (origin + Y/Z)
    auto P = [](const gp_Pnt& o, const gp_Dir& a, double sa, const gp_Dir& b, double sb) {
      return gp_Pnt(o.X() + a.X() * sa + b.X() * sb,
                    o.Y() + a.Y() * sa + b.Y() * sb,
                    o.Z() + a.Z() * sa + b.Z() * sb);
    };
    gp_Pnt p1 = P(ori, dy, offset,  dz, offset);
    gp_Pnt p2 = P(ori, dy, planeSize, dz, offset);
    gp_Pnt p3 = P(ori, dy, planeSize, dz, planeSize);
    gp_Pnt p4 = P(ori, dy, offset,  dz, planeSize);
    
    if (showYZ)
    {
      TopoDS_Wire wireYZ = BRepBuilderAPI_MakeWire(
        BRepBuilderAPI_MakeEdge(p1, p2),
        BRepBuilderAPI_MakeEdge(p2, p3),
        BRepBuilderAPI_MakeEdge(p3, p4),
        BRepBuilderAPI_MakeEdge(p4, p1)
      );
      TopoDS_Face faceYZ = BRepBuilderAPI_MakeFace(wireYZ);
      m_planeYZ = new AIS_Shape(faceYZ);
      if (!m_planeYZ.IsNull()) {
        m_planeYZ->SetColor(colPlane);
        m_planeYZ->SetTransparency(transparency);
        m_planeYZ->SetTransformPersistence(new Graphic3d_TransformPers(Graphic3d_TMF_ZoomPers, gp::Origin()));
        // Set face material to be filled and always visible
        Handle(Prs3d_Drawer) drYZ = m_planeYZ->Attributes();
        if (drYZ.IsNull()) {
          drYZ = new Prs3d_Drawer();
          m_planeYZ->SetAttributes(drYZ);
        }
        drYZ->SetFaceBoundaryDraw(Standard_False);  // Hide edges
        // Force shaded display mode to show filled faces
        m_planeYZ->SetDisplayMode(AIS_Shaded);
        ctx->Display(m_planeYZ, Standard_False);
      }
    }
    
    // Create XZ plane (origin + X/Z)
    gp_Pnt p5 = P(ori, dx, offset,  dz, offset);
    gp_Pnt p6 = P(ori, dx, planeSize, dz, offset);
    gp_Pnt p7 = P(ori, dx, planeSize, dz, planeSize);
    gp_Pnt p8 = P(ori, dx, offset,  dz, planeSize);
    
    if (showXZ)
    {
      TopoDS_Wire wireXZ = BRepBuilderAPI_MakeWire(
        BRepBuilderAPI_MakeEdge(p5, p6),
        BRepBuilderAPI_MakeEdge(p6, p7),
        BRepBuilderAPI_MakeEdge(p7, p8),
        BRepBuilderAPI_MakeEdge(p8, p5)
      );
      TopoDS_Face faceXZ = BRepBuilderAPI_MakeFace(wireXZ);
      m_planeXZ = new AIS_Shape(faceXZ);
      if (!m_planeXZ.IsNull()) {
        m_planeXZ->SetColor(colPlane);
        m_planeXZ->SetTransparency(transparency);
        m_planeXZ->SetTransformPersistence(new Graphic3d_TransformPers(Graphic3d_TMF_ZoomPers, gp::Origin()));
        Handle(Prs3d_Drawer) drXZ = m_planeXZ->Attributes();
        if (drXZ.IsNull()) {
          drXZ = new Prs3d_Drawer();
          m_planeXZ->SetAttributes(drXZ);
        }
        drXZ->SetFaceBoundaryDraw(Standard_False);
        // Force shaded display mode to show filled faces
        m_planeXZ->SetDisplayMode(AIS_Shaded);
        ctx->Display(m_planeXZ, Standard_False);
      }
    }
    
    // Create XY plane (origin + X/Y)
    gp_Pnt p9  = P(ori, dx, offset,    dy, offset);
    gp_Pnt p10 = P(ori, dx, planeSize, dy, offset);
    gp_Pnt p11 = P(ori, dx, planeSize, dy, planeSize);
    gp_Pnt p12 = P(ori, dx, offset,    dy, planeSize);
    
    if (showXY)
    {
      TopoDS_Wire wireXY = BRepBuilderAPI_MakeWire(
        BRepBuilderAPI_MakeEdge(p9, p10),
        BRepBuilderAPI_MakeEdge(p10, p11),
        BRepBuilderAPI_MakeEdge(p11, p12),
        BRepBuilderAPI_MakeEdge(p12, p9)
      );
      TopoDS_Face faceXY = BRepBuilderAPI_MakeFace(wireXY);
      m_planeXY = new AIS_Shape(faceXY);
      if (!m_planeXY.IsNull()) {
        m_planeXY->SetColor(colPlane);
        m_planeXY->SetTransparency(transparency);
        m_planeXY->SetTransformPersistence(new Graphic3d_TransformPers(Graphic3d_TMF_ZoomPers, gp::Origin()));
        Handle(Prs3d_Drawer) drXY = m_planeXY->Attributes();
        if (drXY.IsNull()) {
          drXY = new Prs3d_Drawer();
          m_planeXY->SetAttributes(drXY);
        }
        drXY->SetFaceBoundaryDraw(Standard_False);
        // Force shaded display mode to show filled faces
        m_planeXY->SetDisplayMode(AIS_Shaded);
        ctx->Display(m_planeXY, Standard_False);
      }
    }
    // trihedron already displayed above

    if (topmostOverlay)
    {
      // Keep axes/trihedron in Top overlay (depth-aware),
      // sketches and other 3D overlays go into Topmost (set elsewhere)
      if (!m_trihedron.IsNull()) ctx->SetZLayer(m_trihedron, Graphic3d_ZLayerId_Top);
      if (!m_planeYZ.IsNull()) ctx->SetZLayer(m_planeYZ, Graphic3d_ZLayerId_Top);
      if (!m_planeXZ.IsNull()) ctx->SetZLayer(m_planeXZ, Graphic3d_ZLayerId_Top);
      if (!m_planeXY.IsNull()) ctx->SetZLayer(m_planeXY, Graphic3d_ZLayerId_Top);
      // Origin circle quadrants Z-layers set individually
    }

    // Origin mark: small filled circle with thin outline (Fusion-like)
    if (showOri)
    {
      const Standard_Real r = 10.0;
      gp_Ax2 circAx(ori, dz, dx);
      Handle(Geom_Circle) gC = new Geom_Circle(circAx, r);
      TopoDS_Edge eC = BRepBuilderAPI_MakeEdge(gC);
      TopoDS_Wire wC = BRepBuilderAPI_MakeWire(eC);
      TopoDS_Face fC = BRepBuilderAPI_MakeFace(wC);
      m_originMark = new AIS_Shape(fC);
      {
        Handle(Prs3d_Drawer) dO = m_originMark->Attributes(); if (dO.IsNull()) dO = new Prs3d_Drawer();
        dO->SetColor(Quantity_Color(Quantity_NOC_WHITE));
        dO->SetFaceBoundaryDraw(Standard_True);
        Handle(Prs3d_LineAspect) wa = new Prs3d_LineAspect(Quantity_Color(Quantity_NOC_BLACK), Aspect_TOL_SOLID, 1.5f);
        dO->SetWireAspect(wa);
        m_originMark->SetAttributes(dO);
        m_originMark->SetDisplayMode(AIS_Shaded);
        m_originMark->SetTransformPersistence(new Graphic3d_TransformPers(Graphic3d_TMF_ZoomPers, gp::Origin()));
        m_originMark->SetAutoHilight(true);
      }
      ctx->Display(m_originMark, Standard_False);
      // keep selectable
      if (topmostOverlay) { ctx->SetZLayer(m_originMark, Graphic3d_ZLayerId_Top); }
    }

    // Origin image overlay: small textured quad at Datum origin
    if (showOri)
    {
      Handle(Image_PixMap) spriteImg; // image with RGBA
      QResource res(":/images/circle.png");
      if (res.isValid() && res.size() > 0) {
        Handle(Image_AlienPixMap) imgMem = new Image_AlienPixMap();
        if (imgMem->Load(reinterpret_cast<const Standard_Byte*>(res.data()),
                         static_cast<Standard_Size>(res.size()),
                         TCollection_AsciiString("circle.png"))) {
          spriteImg = imgMem;
        } else {
          QImage qimg; qimg.loadFromData(reinterpret_cast<const uchar*>(res.data()), static_cast<int>(res.size()), "PNG");
          if (!qimg.isNull()) {
            QImage rgba = qimg.convertToFormat(QImage::Format_RGBA8888);
            Handle(Image_PixMap) imgPix = new Image_PixMap();
            if (imgPix->InitZero(Image_Format_RGBA, rgba.width(), rgba.height(), rgba.bytesPerLine())) {
              imgPix->SetTopDown(true);
              const int h = rgba.height();
              for (int y = 0; y < h; ++y) { std::memcpy(imgPix->ChangeRow(y), rgba.constScanLine(y), size_t(rgba.bytesPerLine())); }
              spriteImg = imgPix;
            }
          }
        }
      }
      if (!spriteImg.IsNull()) {
        const Standard_Real s = 48.0;
        Handle(Geom_Plane) plane = new Geom_Plane(gp_Ax3(ori, dz, dx));
        TopoDS_Face f = BRepBuilderAPI_MakeFace(Handle(Geom_Surface)(plane), -s * 0.5, s * 0.5, -s * 0.5, s * 0.5, 1.0e-7);
        m_bgOriginSprite = new AIS_TexturedShape(f);
        m_bgOriginSprite->SetTexturePixMap(spriteImg);
        m_bgOriginSprite->SetTextureMapOn();
        m_bgOriginSprite->SetTextureRepeat(Standard_False, 1.0, 1.0);
        m_bgOriginSprite->DisableTextureModulate();
        // Enable per-pixel alpha blending
        Handle(Prs3d_Drawer) sprDr = m_bgOriginSprite->Attributes(); if (sprDr.IsNull()) sprDr = new Prs3d_Drawer();
        Handle(Prs3d_ShadingAspect) sprShade = sprDr->ShadingAspect(); if (sprShade.IsNull()) { sprShade = new Prs3d_ShadingAspect(); sprDr->SetShadingAspect(sprShade); }
        Handle(Graphic3d_AspectFillArea3d) fillAsp = sprShade->Aspect(); if (fillAsp.IsNull()) fillAsp = new Graphic3d_AspectFillArea3d();
        fillAsp->SetAlphaMode(Graphic3d_AlphaMode_Blend);
        // Push sprite slightly away to avoid failing depth test of the origin mark drawn later
        fillAsp->SetPolygonOffsets(Aspect_POM_Fill, 1.0f, 1.0f);
        sprShade->SetAspect(fillAsp);
        // Semi-transparency for the whole sprite on top of PNG alpha
        sprShade->SetTransparency(0.9f);
        m_bgOriginSprite->SetAttributes(sprDr);
        m_bgOriginSprite->SetDisplayMode(3);
        m_bgOriginSprite->SetAutoHilight(false);
        m_bgOriginSprite->SetTransparency(0.9f);
        m_bgOriginSprite->SetTransformPersistence(new Graphic3d_TransformPers(Graphic3d_TMF_ZoomPers, ori));
        ctx->Display(m_bgOriginSprite, Standard_False);
        // Keep in background with axes
        ctx->SetZLayer(m_bgOriginSprite, Graphic3d_ZLayerId_Default);
        ctx->Deactivate(m_bgOriginSprite);
      }
    }
  }

  void reinstall(const Handle(AIS_InteractiveContext)& ctx)
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
    // Origin circle quadrants are managed separately
  }

  void erase(const Handle(AIS_InteractiveContext)& ctx)
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
    // Origin circle quadrants are managed separately
  }

  Handle(AIS_InteractiveObject) bgAxisX() const { return m_bgAxisX; }
  Handle(AIS_InteractiveObject) bgAxisY() const { return m_bgAxisY; }
  Handle(AIS_Trihedron) trihedron() const { return m_trihedron; }

  void setAxisExtents(const Handle(AIS_InteractiveContext)& ctx, Standard_Real halfX, Standard_Real halfY)
  {
    if (ctx.IsNull()) return;
    if (!m_bgAxisX.IsNull()) ctx->Erase(m_bgAxisX, Standard_False);
    if (!m_bgAxisY.IsNull()) ctx->Erase(m_bgAxisY, Standard_False);
    m_bgAxisX = new AIS_Shape(BRepBuilderAPI_MakeEdge(gp_Pnt(-halfX, 0.0, 0.0), gp_Pnt(halfX, 0.0, 0.0)));
    m_bgAxisY = new AIS_Shape(BRepBuilderAPI_MakeEdge(gp_Pnt(0.0, -halfY, 0.0), gp_Pnt(0.0, halfY, 0.0)));
    const Quantity_Color colX(1.00, 0.20, 0.20, Quantity_TOC_sRGB);
    const Quantity_Color colY(0.25, 0.90, 0.25, Quantity_TOC_sRGB);
    Handle(Prs3d_Drawer) dx = new Prs3d_Drawer(); dx->SetLineAspect(new Prs3d_LineAspect(colX, Aspect_TOL_SOLID, 2.0f));
    m_bgAxisX->SetAttributes(dx); m_bgAxisX->SetColor(colX); m_bgAxisX->SetDisplayMode(AIS_WireFrame); m_bgAxisX->SetAutoHilight(false);
    Handle(Prs3d_Drawer) dy = new Prs3d_Drawer(); dy->SetLineAspect(new Prs3d_LineAspect(colY, Aspect_TOL_SOLID, 2.0f));
    m_bgAxisY->SetAttributes(dy); m_bgAxisY->SetColor(colY); m_bgAxisY->SetDisplayMode(AIS_WireFrame); m_bgAxisY->SetAutoHilight(false);
    ctx->Display(m_bgAxisX, Standard_False);
    ctx->Display(m_bgAxisY, Standard_False);
    ctx->Deactivate(m_bgAxisX);
    ctx->Deactivate(m_bgAxisY);
    ctx->SetZLayer(m_bgAxisX, Graphic3d_ZLayerId_Default);
    ctx->SetZLayer(m_bgAxisY, Graphic3d_ZLayerId_Default);
  }

private:
  Handle(AIS_InteractiveObject) m_bgAxisX;
  Handle(AIS_InteractiveObject) m_bgAxisY;
  Handle(AIS_Trihedron)          m_trihedron;

  Handle(AIS_Shape)     m_planeYZ;
  Handle(AIS_Shape)     m_planeXZ;
  Handle(AIS_Shape)     m_planeXY;
  Handle(AIS_Shape)          m_originMark;
  Handle(AIS_TexturedShape)  m_bgOriginSprite;
};

#endif
