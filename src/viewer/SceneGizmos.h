// Axes + Trihedron overlay manager
#ifndef _SceneGizmos_HeaderFile
#define _SceneGizmos_HeaderFile

#include <AIS_InteractiveContext.hxx>
#include <AIS_Line.hxx>
#include <AIS_Plane.hxx>
#include <AIS_Shape.hxx>
#include <Geom_Plane.hxx>
#include <AIS_Trihedron.hxx>
#include <Geom_Axis2Placement.hxx>
#include <Prs3d_DatumAspect.hxx>
#include <Prs3d_LineAspect.hxx>
#include <Graphic3d_ZLayerId.hxx>
#include <Quantity_Color.hxx>
#include <Geom_Line.hxx>
#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
#include <gp.hxx>
#include <gp_Ax3.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>
#include <Aspect_TypeOfFacingModel.hxx>
#include <Graphic3d_MaterialAspect.hxx>
#include <Graphic3d_NameOfMaterial.hxx>
#include <Aspect_InteriorStyle.hxx>

class SceneGizmos
{
public:
  SceneGizmos() = default;

  void install(const Handle(AIS_InteractiveContext)& ctx,
               const Handle(Geom_Axis2Placement)&    originA2,
               Standard_Boolean                      topmostOverlay = Standard_True)
  {
    if (ctx.IsNull()) return;

    // Colors
    const Quantity_Color colX(1.00, 0.20, 0.20, Quantity_TOC_sRGB);
    const Quantity_Color colY(0.25, 0.90, 0.25, Quantity_TOC_sRGB);
    const Quantity_Color colZ(0.25, 0.45, 1.00, Quantity_TOC_sRGB);
    const Quantity_Color colPlane(0.95, 0.85, 0.7, Quantity_TOC_sRGB); // Soft peachy beige

    // Add small filled planes between axes in positive quadrant, like Fusion 360
    const Standard_Real planeSize = 150.0;  // Larger size for better visibility
    const Standard_Real offset = 30.0;       // offset from origin along each axis
    const Standard_Real transparency = 0.3f; // 50% transparency for semi-transparent effect

    const Standard_Real L = 1000.0;
    Handle(Geom_Line) gX = new Geom_Line(gp_Pnt(-L, 0.0, 0.0), gp_Dir(1.0, 0.0, 0.0));
    Handle(Geom_Line) gY = new Geom_Line(gp_Pnt(0.0, -L, 0.0), gp_Dir(0.0, 1.0, 0.0));
    Handle(Geom_Line) gZ = new Geom_Line(gp_Pnt(0.0, 0.0, -L), gp_Dir(0.0, 0.0, 1.0));
    m_axisX = new AIS_Line(gX);
    m_axisY = new AIS_Line(gY);
    m_axisZ = new AIS_Line(gZ);

    Handle(Prs3d_Drawer) dx = m_axisX->Attributes(); if (dx.IsNull()) dx = new Prs3d_Drawer();
    dx->SetLineAspect(new Prs3d_LineAspect(colX, Aspect_TOL_SOLID, 3.0f)); m_axisX->SetAttributes(dx);
    Handle(Prs3d_Drawer) dy = m_axisY->Attributes(); if (dy.IsNull()) dy = new Prs3d_Drawer();
    dy->SetLineAspect(new Prs3d_LineAspect(colY, Aspect_TOL_SOLID, 3.0f)); m_axisY->SetAttributes(dy);
    Handle(Prs3d_Drawer) dz = m_axisZ->Attributes(); if (dz.IsNull()) dz = new Prs3d_Drawer();
    dz->SetLineAspect(new Prs3d_LineAspect(colZ, Aspect_TOL_SOLID, 3.0f)); m_axisZ->SetAttributes(dz);

    m_trihedron = new AIS_Trihedron(originA2);
    Handle(Prs3d_Drawer) trD = m_trihedron->Attributes(); if (trD.IsNull()) trD = new Prs3d_Drawer();
    Handle(Prs3d_DatumAspect) dAsp = trD->DatumAspect(); if (dAsp.IsNull()) dAsp = new Prs3d_DatumAspect();
    dAsp->SetAxisLength(300.0, 300.0, 300.0);
    trD->SetDatumAspect(dAsp);
    m_trihedron->SetAttributes(trD);
    m_trihedron->SetDatumPartColor(Prs3d_DatumParts_XAxis, colX);
    m_trihedron->SetDatumPartColor(Prs3d_DatumParts_YAxis, colY);
    m_trihedron->SetDatumPartColor(Prs3d_DatumParts_ZAxis, colZ);
    m_trihedron->SetArrowColor(Prs3d_DatumParts_XAxis, colX);
    m_trihedron->SetArrowColor(Prs3d_DatumParts_YAxis, colY);
    m_trihedron->SetArrowColor(Prs3d_DatumParts_ZAxis, colZ);
    m_trihedron->SetTransformPersistence(new Graphic3d_TransformPers(Graphic3d_TMF_ZoomPers, gp::Origin()));
    
    // Create YZ plane (between Y and Z axes, starting from offset on both Y and Z)
    gp_Pnt p1(0, offset, offset);
    gp_Pnt p2(0, planeSize, offset);
    gp_Pnt p3(0, planeSize, planeSize);
    gp_Pnt p4(0, offset, planeSize);
    
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
    
    // Create XZ plane (between X and Z axes, starting from offset on both X and Z)
    gp_Pnt p5(offset, 0, offset);
    gp_Pnt p6(planeSize, 0, offset);
    gp_Pnt p7(planeSize, 0, planeSize);
    gp_Pnt p8(offset, 0, planeSize);
    
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
    
    // Create XY plane (between X and Y axes, offset along Z)
    gp_Pnt p9(offset, offset, 0);
    gp_Pnt p10(planeSize, offset, 0);
    gp_Pnt p11(planeSize, planeSize, 0);
    gp_Pnt p12(offset, planeSize, 0);
    
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

    ctx->Display(m_axisX, Standard_False);
    ctx->Display(m_axisY, Standard_False);
    ctx->Display(m_axisZ, Standard_False);
    ctx->Display(m_trihedron, Standard_False);

    if (topmostOverlay)
    {
      // Keep axes/trihedron in Top overlay (depth-aware),
      // sketches and other 3D overlays go into Topmost (set elsewhere)
      ctx->SetZLayer(m_axisX, Graphic3d_ZLayerId_Top);
      ctx->SetZLayer(m_axisY, Graphic3d_ZLayerId_Top);
      ctx->SetZLayer(m_axisZ, Graphic3d_ZLayerId_Top);
      ctx->SetZLayer(m_trihedron, Graphic3d_ZLayerId_Top);
      if (!m_planeYZ.IsNull()) ctx->SetZLayer(m_planeYZ, Graphic3d_ZLayerId_Top);
      if (!m_planeXZ.IsNull()) ctx->SetZLayer(m_planeXZ, Graphic3d_ZLayerId_Top);
      if (!m_planeXY.IsNull()) ctx->SetZLayer(m_planeXY, Graphic3d_ZLayerId_Top);
    }
  }

  void reinstall(const Handle(AIS_InteractiveContext)& ctx)
  {
    if (ctx.IsNull()) return;
    if (!m_axisX.IsNull()) ctx->Display(m_axisX, Standard_False);
    if (!m_axisY.IsNull()) ctx->Display(m_axisY, Standard_False);
    if (!m_axisZ.IsNull()) ctx->Display(m_axisZ, Standard_False);
    if (!m_trihedron.IsNull()) ctx->Display(m_trihedron, Standard_False);
    if (!m_planeYZ.IsNull()) ctx->Display(m_planeYZ, Standard_False);
    if (!m_planeXZ.IsNull()) ctx->Display(m_planeXZ, Standard_False);
    if (!m_planeXY.IsNull()) ctx->Display(m_planeXY, Standard_False);
  }

  void erase(const Handle(AIS_InteractiveContext)& ctx)
  {
    if (ctx.IsNull()) return;
    if (!m_axisX.IsNull()) ctx->Erase(m_axisX, Standard_False);
    if (!m_axisY.IsNull()) ctx->Erase(m_axisY, Standard_False);
    if (!m_axisZ.IsNull()) ctx->Erase(m_axisZ, Standard_False);
    if (!m_trihedron.IsNull()) ctx->Erase(m_trihedron, Standard_False);
    if (!m_planeYZ.IsNull()) ctx->Erase(m_planeYZ, Standard_False);
    if (!m_planeXZ.IsNull()) ctx->Erase(m_planeXZ, Standard_False);
    if (!m_planeXY.IsNull()) ctx->Erase(m_planeXY, Standard_False);
  }

  Handle(AIS_Line) axisX() const { return m_axisX; }
  Handle(AIS_Line) axisY() const { return m_axisY; }
  Handle(AIS_Line) axisZ() const { return m_axisZ; }
  Handle(AIS_Trihedron) trihedron() const { return m_trihedron; }

private:
  Handle(AIS_Line)      m_axisX;
  Handle(AIS_Line)      m_axisY;
  Handle(AIS_Line)      m_axisZ;
  Handle(AIS_Trihedron) m_trihedron;

  Handle(AIS_Shape)     m_planeYZ;
  Handle(AIS_Shape)     m_planeXZ;
  Handle(AIS_Shape)     m_planeXY;
};

#endif
