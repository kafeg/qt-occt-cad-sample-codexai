// Axes + Trihedron overlay manager
#ifndef _SceneGizmos_HeaderFile
#define _SceneGizmos_HeaderFile

#include <AIS_InteractiveContext.hxx>
#include <AIS_Line.hxx>
#include <AIS_Trihedron.hxx>
#include <Geom_Axis2Placement.hxx>
#include <Prs3d_DatumAspect.hxx>
#include <Prs3d_LineAspect.hxx>
#include <Graphic3d_ZLayerId.hxx>
#include <Quantity_Color.hxx>

class SceneGizmos
{
public:
  SceneGizmos() = default;

  void install(const Handle(AIS_InteractiveContext)& ctx,
               const Handle(Geom_Axis2Placement)&    originA2,
               Standard_Boolean                      topmostOverlay = Standard_True)
  {
    if (ctx.IsNull()) return;
    const Quantity_Color colX(1.00, 0.20, 0.20, Quantity_TOC_sRGB);
    const Quantity_Color colY(0.25, 0.90, 0.25, Quantity_TOC_sRGB);
    const Quantity_Color colZ(0.25, 0.45, 1.00, Quantity_TOC_sRGB);

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
    dAsp->SetAxisLength(24.0, 24.0, 24.0);
    trD->SetDatumAspect(dAsp);
    m_trihedron->SetAttributes(trD);
    m_trihedron->SetDatumPartColor(Prs3d_DatumParts_XAxis, colX);
    m_trihedron->SetDatumPartColor(Prs3d_DatumParts_YAxis, colY);
    m_trihedron->SetDatumPartColor(Prs3d_DatumParts_ZAxis, colZ);
    m_trihedron->SetArrowColor(Prs3d_DatumParts_XAxis, colX);
    m_trihedron->SetArrowColor(Prs3d_DatumParts_YAxis, colY);
    m_trihedron->SetArrowColor(Prs3d_DatumParts_ZAxis, colZ);

    ctx->Display(m_axisX, Standard_False);
    ctx->Display(m_axisY, Standard_False);
    ctx->Display(m_axisZ, Standard_False);
    ctx->Display(m_trihedron, Standard_False);

    if (topmostOverlay)
    {
      ctx->SetZLayer(m_axisX, Graphic3d_ZLayerId_Topmost);
      ctx->SetZLayer(m_axisY, Graphic3d_ZLayerId_Topmost);
      ctx->SetZLayer(m_axisZ, Graphic3d_ZLayerId_Topmost);
      ctx->SetZLayer(m_trihedron, Graphic3d_ZLayerId_Topmost);
    }
  }

  void reinstall(const Handle(AIS_InteractiveContext)& ctx)
  {
    if (ctx.IsNull()) return;
    if (!m_axisX.IsNull()) ctx->Display(m_axisX, Standard_False);
    if (!m_axisY.IsNull()) ctx->Display(m_axisY, Standard_False);
    if (!m_axisZ.IsNull()) ctx->Display(m_axisZ, Standard_False);
    if (!m_trihedron.IsNull()) ctx->Display(m_trihedron, Standard_False);
  }

  void erase(const Handle(AIS_InteractiveContext)& ctx)
  {
    if (ctx.IsNull()) return;
    if (!m_axisX.IsNull()) ctx->Erase(m_axisX, Standard_False);
    if (!m_axisY.IsNull()) ctx->Erase(m_axisY, Standard_False);
    if (!m_axisZ.IsNull()) ctx->Erase(m_axisZ, Standard_False);
    if (!m_trihedron.IsNull()) ctx->Erase(m_trihedron, Standard_False);
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
};

#endif

