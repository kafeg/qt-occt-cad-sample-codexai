#pragma once

#include <gp_Pnt2d.hxx>
#include <TopoDS_Wire.hxx>

#include <cstddef>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

#include <DocumentItem.h>
#include <Standard_DefineHandle.hxx>
#include <Standard_Transient.hxx>

// Lightweight 2D sketch container with simple constraint handling
// - Stores lines and circular arcs in 2D (gp_Pnt2d)
// - Supports Coincident endpoint constraints (union-find based)
// - Computes wires as connected sets of curves by shared endpoints
class Sketch : public DocumentItem
{
  DEFINE_STANDARD_RTTIEXT(Sketch, DocumentItem)
public:
  using CurveId = int;

  enum class CurveType
  {
    Line,
    Arc
  };

  struct Line
  {
    gp_Pnt2d p1;
    gp_Pnt2d p2;
  };

  // Arc defined by center and its start/end points on the circle
  struct Arc
  {
    gp_Pnt2d center;
    gp_Pnt2d p1;  // start point
    gp_Pnt2d p2;  // end point
    bool clockwise{false};
  };

  struct Curve
  {
    CurveType type{CurveType::Line};
    Line line{};  // valid when type == Line
    Arc arc{};    // valid when type == Arc
  };

  // EndpointRef selects one endpoint of a curve: index 0 or 1
  // - For Line: 0 => p1, 1 => p2
  // - For Arc: 0 => p1, 1 => p2
  struct EndpointRef
  {
    CurveId curve{ -1 };
    int endIndex{0};
  };

  enum class ConstraintType
  {
    Coincident,
  };

  struct Constraint
  {
    ConstraintType type{ConstraintType::Coincident};
    EndpointRef a{};
    EndpointRef b{};
  };

  struct Wire
  {
    std::vector<CurveId> curves;  // unordered list of connected curves
  };

  struct OrderedCurve
  {
    CurveId id{ -1 };
    bool reversed{false};
  };

  using OrderedPath = std::vector<OrderedCurve>;

public:
  Sketch() = default;
  explicit Sketch(DocumentItem::Id existingId) : DocumentItem(existingId) {}

  // DocumentItem
  Kind kind() const override { return Kind::Sketch; }
  std::string serialize() const override;
  void        deserialize(const std::string& data) override;

  // Add primitives
  CurveId addLine(const gp_Pnt2d& a, const gp_Pnt2d& b);
  CurveId addArc(const gp_Pnt2d& center, const gp_Pnt2d& a, const gp_Pnt2d& b, bool clockwise);

  // Add constraints
  void addCoincident(const EndpointRef& a, const EndpointRef& b);

  // Solve currently supports Coincident endpoint constraints only
  void solveConstraints(double tol = 1.0e-9);

  // Compute wires by endpoint connectivity (after constraints solved)
  std::vector<Wire> computeWires(double tol = 1.0e-9) const;

  // Compute ordered paths for each connected component (may split into multiple paths if branching)
  std::vector<OrderedPath> computeOrderedPaths(double tol = 1.0e-9) const;

  // Export ordered paths as OCCT wires in XY plane (Z=0)
  std::vector<TopoDS_Wire> toOcctWires(double tol = 1.0e-9) const;

  // Access
  const std::vector<Curve>& curves() const { return curves_; }
  const std::vector<Constraint>& constraints() const { return constraints_; }

private:
  // Endpoint index into a flattened list (each curve contributes two endpoints)
  std::size_t endpointKey(const EndpointRef& r) const { return static_cast<std::size_t>(r.curve) * 2 + (r.endIndex & 1); }
  gp_Pnt2d getEndpoint(const EndpointRef& r) const;
  void setEndpoint(const EndpointRef& r, const gp_Pnt2d& p);

  // Union-Find for endpoint clustering
  void ufInit(std::size_t n);
  std::size_t ufFind(std::size_t i) const;
  void ufUnion(std::size_t a, std::size_t b);

private:
  std::vector<Curve> curves_{};
  std::vector<Constraint> constraints_{};

  // mutable because computeWires groups by endpoint clusters without mutating geometry
  mutable std::vector<std::size_t> uf_parent_{};
};

// Enable OCCT handle for Sketch
DEFINE_STANDARD_HANDLE(Sketch, DocumentItem)
