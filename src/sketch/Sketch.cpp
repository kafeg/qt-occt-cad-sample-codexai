#include "Sketch.h"
#include <DocumentItem.h>
#include <sstream>

IMPLEMENT_STANDARD_RTTIEXT(Sketch, DocumentItem)

namespace {
const bool kSketchReg = [](){
  DocumentItem::registerFactory(DocumentItem::Kind::Sketch, [](){ return std::shared_ptr<DocumentItem>(new Sketch()); });
  return true;
}();
}

#include <cmath>
#include <deque>
#include <unordered_set>
#include <algorithm>
#include <limits>

#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <GC_MakeArcOfCircle.hxx>
#include <Geom_Circle.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <TopExp_Explorer.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <gp_Ax2.hxx>
#include <gp_Circ.hxx>
#include <gp_Pnt.hxx>

namespace
{
inline bool nearlyEqual(double a, double b, double tol)
{
  return std::abs(a - b) <= tol;
}

// Simple 2D k-d tree for range queries (axis-aligned rectangle)
// Used to find near-coincident endpoints in O(log n + k)
namespace {
struct KDPoint
{
  double x{0.0};
  double y{0.0};
  int    id{-1}; // external identifier (endpoint index)
};

struct KDNode
{
  int point{-1};     // index into pts
  int left{-1};
  int right{-1};
  int axis{0};       // 0 = X, 1 = Y
  double minx{0}, maxx{0}, miny{0}, maxy{0};
};

class KDTree2D
{
public:
  explicit KDTree2D(std::vector<KDPoint> pts)
      : pts_(std::move(pts))
  {
    if (!pts_.empty())
    {
      std::vector<int> idx(pts_.size());
      for (int i = 0; i < static_cast<int>(pts_.size()); ++i) idx[i] = i;
      nodes_.reserve(pts_.size());
      root_ = build(idx.begin(), idx.end(), /*depth*/ 0);
    }
  }

  // Visit all points within axis-aligned rectangle [xmin,xmax] x [ymin,ymax]
  template <typename Visitor>
  void rangeQuery(double xmin, double ymin, double xmax, double ymax, Visitor visit) const
  {
    if (root_ != -1)
      rangeQueryRec(root_, xmin, ymin, xmax, ymax, visit);
  }

private:
  using It = std::vector<int>::iterator;

  int build(It begin, It end, int depth)
  {
    if (begin == end) return -1;
    int axis = depth % 2;
    auto mid = begin + (std::distance(begin, end) / 2);
    std::nth_element(begin, mid, end, [&](int a, int b){
      return axis == 0 ? (pts_[a].x < pts_[b].x) : (pts_[a].y < pts_[b].y);
    });

    int nodeIndex = static_cast<int>(nodes_.size());
    nodes_.push_back(KDNode{});
    KDNode& node = nodes_.back();
    node.axis = axis;
    node.point = *mid;

    // Build subtrees
    node.left = build(begin, mid, depth + 1);
    node.right = build(mid + 1, end, depth + 1);

    // Compute bounding box
    const KDPoint& p = pts_[node.point];
    node.minx = node.maxx = p.x;
    node.miny = node.maxy = p.y;
    if (node.left != -1)
    {
      const KDNode& L = nodes_[node.left];
      node.minx = std::min(node.minx, L.minx);
      node.maxx = std::max(node.maxx, L.maxx);
      node.miny = std::min(node.miny, L.miny);
      node.maxy = std::max(node.maxy, L.maxy);
    }
    if (node.right != -1)
    {
      const KDNode& R = nodes_[node.right];
      node.minx = std::min(node.minx, R.minx);
      node.maxx = std::max(node.maxx, R.maxx);
      node.miny = std::min(node.miny, R.miny);
      node.maxy = std::max(node.maxy, R.maxy);
    }

    return nodeIndex;
  }

  template <typename Visitor>
  void rangeQueryRec(int nodeIndex, double xmin, double ymin, double xmax, double ymax, Visitor visit) const
  {
    const KDNode& node = nodes_[nodeIndex];
    // If node bounding box does not intersect query rect, prune
    if (node.maxx < xmin || node.minx > xmax || node.maxy < ymin || node.miny > ymax)
      return;

    const KDPoint& p = pts_[node.point];
    if (p.x >= xmin && p.x <= xmax && p.y >= ymin && p.y <= ymax)
      visit(p);

    if (node.left != -1)  rangeQueryRec(node.left, xmin, ymin, xmax, ymax, visit);
    if (node.right != -1) rangeQueryRec(node.right, xmin, ymin, xmax, ymax, visit);
  }

  std::vector<KDPoint> pts_;
  std::vector<KDNode>  nodes_;
  int root_{-1};
};
} // namespace

inline bool samePoint(const gp_Pnt2d& a, const gp_Pnt2d& b, double tol)
{
  return nearlyEqual(a.X(), b.X(), tol) && nearlyEqual(a.Y(), b.Y(), tol);
}
}  // namespace

Sketch::CurveId Sketch::addLine(const gp_Pnt2d& a, const gp_Pnt2d& b)
{
  Curve c;
  c.type = CurveType::Line;
  c.line = Line{a, b};
  curves_.push_back(c);
  return static_cast<CurveId>(curves_.size() - 1);
}

Sketch::CurveId Sketch::addLineAuto(const gp_Pnt2d& aIn, const gp_Pnt2d& bIn, double tol)
{
  auto sqr = [](double v){ return v*v; };
  auto dist2 = [&](const gp_Pnt2d& p, const gp_Pnt2d& q) {
    return sqr(p.X() - q.X()) + sqr(p.Y() - q.Y());
  };

  // Build KDTree of existing endpoints for snapping
  std::vector<KDPoint> pts;
  pts.reserve(curves_.size() * 2);
  for (std::size_t i = 0; i < curves_.size(); ++i)
  {
    for (int e = 0; e < 2; ++e)
    {
      EndpointRef r{static_cast<int>(i), e};
      const auto p = getEndpoint(r);
      pts.push_back(KDPoint{p.X(), p.Y(), static_cast<int>(endpointKey(r))});
    }
  }
  gp_Pnt2d a = aIn;
  gp_Pnt2d b = bIn;
  std::optional<EndpointRef> snapA, snapB;
  if (!pts.empty())
  {
    KDTree2D kdt(std::move(pts));
    auto snapToNearest = [&](const gp_Pnt2d& p, std::optional<EndpointRef>& outRef, gp_Pnt2d& outP){
      const double xmin = p.X() - tol, xmax = p.X() + tol;
      const double ymin = p.Y() - tol, ymax = p.Y() + tol;
      double bestD2 = std::numeric_limits<double>::infinity();
      int bestId = -1;
      kdt.rangeQuery(xmin, ymin, xmax, ymax, [&](const KDPoint& q){
        gp_Pnt2d q2(q.x, q.y);
        double d2 = dist2(p, q2);
        if (d2 < bestD2)
        {
          bestD2 = d2; bestId = q.id;
        }
      });
      if (bestId >= 0 && bestD2 <= tol*tol)
      {
        int curveIdx = bestId / 2;
        int endIdx   = bestId % 2;
        outRef = EndpointRef{curveIdx, endIdx};
        outP   = getEndpoint(*outRef);
      }
    };
    snapToNearest(a, snapA, a);
    snapToNearest(b, snapB, b);
  }
  // Do not snap to auxiliary sketch points; keep only endpoint snapping to existing curve endpoints

  // Add the line (possibly with snapped endpoints)
  const CurveId newId = addLine(a, b);

  // Add explicit coincident constraints if snapped
  if (snapA)
    addCoincident(EndpointRef{newId, 0}, *snapA);
  if (snapB)
    addCoincident(EndpointRef{newId, 1}, *snapB);

  // Helper: split an existing line at point P if P falls inside the segment (T-junction)
  auto splitAtPointIfInside = [&](int curveIdx, const gp_Pnt2d& P) -> bool {
    if (curveIdx < 0 || curveIdx >= static_cast<int>(curves_.size())) return false;
    auto& c = curves_[static_cast<std::size_t>(curveIdx)];
    if (c.type != CurveType::Line) return false;
    const gp_Pnt2d A = c.line.p1;
    const gp_Pnt2d B = c.line.p2;
    const double vx = B.X() - A.X();
    const double vy = B.Y() - A.Y();
    const double len2 = vx*vx + vy*vy;
    if (len2 <= std::numeric_limits<double>::epsilon()) return false; // degenerate
    const double t = ((P.X() - A.X())*vx + (P.Y() - A.Y())*vy) / len2;
    // Check strictly interior, with margin tol to avoid endpoint duplicates
    if (t <= tol || t >= 1.0 - tol) return false;
    // Closest point on AB
    gp_Pnt2d X(A.X() + t*vx, A.Y() + t*vy);
    // Ensure the perpendicular distance is within tol
    const double d2 = (P.X() - X.X())*(P.X() - X.X()) + (P.Y() - X.Y())*(P.Y() - X.Y());
    if (d2 > tol*tol) return false;

    // Perform split: replace curveIdx with [A-X], append [X-B]
    Curve first; first.type = CurveType::Line; first.line = Line{A, X};
    Curve second; second.type = CurveType::Line; second.line = Line{X, B};

    // Update constraints referencing old endpoint 1 (B) to point to the new second segment's endpoint 1
    const int newSecondIdx = static_cast<int>(curves_.size());
    for (auto& k : constraints_)
    {
      if (k.a.curve == curveIdx && k.a.endIndex == 1) { k.a.curve = newSecondIdx; }
      if (k.b.curve == curveIdx && k.b.endIndex == 1) { k.b.curve = newSecondIdx; }
    }

    curves_[static_cast<std::size_t>(curveIdx)] = first;
    curves_.push_back(second);

    // Add coincident between the shared split point endpoints for stability
    addCoincident(EndpointRef{curveIdx, 1}, EndpointRef{newSecondIdx, 0});
    return true;
  };

  // For each new endpoint, if it falls on an existing segment's interior (within tol), split that segment
  const gp_Pnt2d NA = getEndpoint(EndpointRef{newId, 0});
  const gp_Pnt2d NB = getEndpoint(EndpointRef{newId, 1});

  // Try split against all existing segments prior to the new one
  for (int i = 0; i < newId; ++i)
  {
    if (splitAtPointIfInside(i, NA))
    {
      // Add coincident between new endpoint and the split node (the first segment's endpoint 1)
      addCoincident(EndpointRef{newId, 0}, EndpointRef{i, 1});
      break; // split only one segment per endpoint
    }
  }
  for (int i = 0; i < newId; ++i)
  {
    if (splitAtPointIfInside(i, NB))
    {
      addCoincident(EndpointRef{newId, 1}, EndpointRef{i, 1});
      break;
    }
  }

  // Create intersection points with other line segments (proper crossings, not at endpoints)
  auto tryAddIntersectionWith = [&](int i){
    const auto& ci = curves_[static_cast<std::size_t>(i)];
    if (ci.type != CurveType::Line) return;
    const gp_Pnt2d A = ci.line.p1;
    const gp_Pnt2d B = ci.line.p2;
    const gp_Pnt2d C = NA;
    const gp_Pnt2d D = NB;
    const double x1=A.X(), y1=A.Y(), x2=B.X(), y2=B.Y();
    const double x3=C.X(), y3=C.Y(), x4=D.X(), y4=D.Y();
    const double den = (x1-x2)*(y3-y4) - (y1-y2)*(x3-x4);
    if (std::abs(den) <= 1.0e-18) return; // parallel/coincident
    const double t = ((x1-x3)*(y3-y4) - (y1-y3)*(x3-x4)) / den;
    const double u = ((x1-x3)*(y1-y2) - (y1-y3)*(x1-x2)) / den;
    if (t <= tol || t >= 1.0 - tol || u <= tol || u >= 1.0 - tol) return; // ignore near endpoints
    const gp_Pnt2d P(x1 + t*(x2-x1), y1 + t*(y2-y1));
    // Deduplicate by proximity to existing points
    for (const auto& q : points_) {
      const double dx = q.X() - P.X();
      const double dy = q.Y() - P.Y();
      if (dx*dx + dy*dy <= tol*tol) return;
    }
    addPoint(P);
  };
  for (int i = 0; i < newId; ++i) { tryAddIntersectionWith(i); }

  // Full cross-intersection splitting of both existing lines and the new line
  {
    struct CrossHit { int curveIdx; double uNew; gp_Pnt2d P; };
    std::vector<CrossHit> hits;
    const double x1=NA.X(), y1=NA.Y(), x2=NB.X(), y2=NB.Y();
    for (int i = 0; i < newId; ++i)
    {
      const auto& ci = curves_[static_cast<std::size_t>(i)];
      if (ci.type != CurveType::Line) continue;
      const double x3=ci.line.p1.X(), y3=ci.line.p1.Y();
      const double x4=ci.line.p2.X(), y4=ci.line.p2.Y();
      const double den = (x1-x2)*(y3-y4) - (y1-y2)*(x3-x4);
      if (std::abs(den) <= 1.0e-18) continue; // parallel/coincident
      const double t = ((x1-x3)*(y3-y4) - (y1-y3)*(x3-x4)) / den; // param on new [0,1]
      const double u = ((x1-x3)*(y1-y2) - (y1-y3)*(x1-x2)) / den; // param on existing [0,1]
      if (t <= tol || t >= 1.0 - tol || u <= tol || u >= 1.0 - tol) continue; // ignore near endpoints
      const gp_Pnt2d P(x1 + t*(x2-x1), y1 + t*(y2-y1));
      hits.push_back(CrossHit{i, t, P});
    }

    std::sort(hits.begin(), hits.end(), [](const CrossHit& a, const CrossHit& b){ return a.uNew < b.uNew; });
    std::vector<CrossHit> uniqueHits; uniqueHits.reserve(hits.size());
    for (const auto& h : hits)
    {
      if (uniqueHits.empty() || std::abs(h.uNew - uniqueHits.back().uNew) > tol)
        uniqueHits.push_back(h);
    }

    if (!uniqueHits.empty())
    {
      // Split existing
      for (const auto& h : uniqueHits) { splitAtPointIfInside(h.curveIdx, h.P); }

      // Split the new line at all intersection points
      std::vector<gp_Pnt2d> cuts; cuts.reserve(uniqueHits.size() + 2);
      cuts.push_back(NA); for (const auto& h : uniqueHits) cuts.push_back(h.P); cuts.push_back(NB);

      std::vector<int> newSegIds; newSegIds.reserve(cuts.size() - 1);
      curves_[static_cast<std::size_t>(newId)].type = CurveType::Line;
      curves_[static_cast<std::size_t>(newId)].line = Line{cuts[0], cuts[1]};
      newSegIds.push_back(newId);
      int lastId = newId;
      for (std::size_t k = 1; k + 1 < cuts.size(); ++k)
      {
        Curve seg; seg.type = CurveType::Line; seg.line = Line{cuts[k], cuts[k+1]};
        curves_.push_back(seg);
        int segId = static_cast<int>(curves_.size()) - 1;
        addCoincident(EndpointRef{lastId, 1}, EndpointRef{segId, 0});
        newSegIds.push_back(segId);
        lastId = segId;
      }

      // Tie each interior joint of the new line to the nearest existing endpoint cluster
      std::vector<KDPoint> eps; eps.reserve(curves_.size() * 2);
      for (std::size_t i = 0; i < curves_.size(); ++i)
        for (int e = 0; e < 2; ++e)
        {
          EndpointRef r{static_cast<int>(i), e}; const auto p = getEndpoint(r);
          eps.push_back(KDPoint{p.X(), p.Y(), static_cast<int>(endpointKey(r))});
        }
      KDTree2D eKdt(std::move(eps));
      for (std::size_t j = 1; j + 1 < cuts.size(); ++j)
      {
        const gp_Pnt2d& P = cuts[j];
        const double xmin = P.X() - tol, xmax = P.X() + tol;
        const double ymin = P.Y() - tol, ymax = P.Y() + tol;
        int bestId = -1; double bestD2 = std::numeric_limits<double>::infinity();
        eKdt.rangeQuery(xmin, ymin, xmax, ymax, [&](const KDPoint& q){
          gp_Pnt2d q2(q.x, q.y); double d2 = dist2(P, q2);
          if (d2 < bestD2) { bestD2 = d2; bestId = q.id; }
        });
        if (bestId >= 0 && bestD2 <= tol*tol)
        {
          int nearCurve = bestId / 2; int nearEnd = bestId % 2;
          addCoincident(EndpointRef{newSegIds[j-1], 1}, EndpointRef{nearCurve, nearEnd});
        }
      }
    }
  }

  return newId;
}

Sketch::CurveId Sketch::addArc(const gp_Pnt2d& center, const gp_Pnt2d& a, const gp_Pnt2d& b, bool clockwise)
{
  Curve c;
  c.type = CurveType::Arc;
  c.arc = Arc{center, a, b, clockwise};
  curves_.push_back(c);
  return static_cast<CurveId>(curves_.size() - 1);
}

int Sketch::addPoint(const gp_Pnt2d& p)
{
  points_.push_back(p);
  return static_cast<int>(points_.size() - 1);
}

void Sketch::addCoincident(const EndpointRef& a, const EndpointRef& b)
{
  constraints_.push_back(Constraint{ConstraintType::Coincident, a, b});
}

void Sketch::solveConstraints(double tol)
{
  // Initialize union-find for all endpoints
  const std::size_t epCount = curves_.size() * 2;
  ufInit(epCount);

  // First, union existing near-coincident endpoints using spatial index (k-d tree)
  if (!curves_.empty())
  {
    std::vector<KDPoint> pts;
    pts.reserve(epCount);
    for (std::size_t i = 0; i < curves_.size(); ++i)
    {
      for (int e = 0; e < 2; ++e)
      {
        EndpointRef r{static_cast<int>(i), e};
        const auto p = getEndpoint(r);
        pts.push_back(KDPoint{p.X(), p.Y(), static_cast<int>(endpointKey(r))});
      }
    }
    KDTree2D kdt(std::move(pts));

    // For each endpoint, query its neighborhood rectangle [x±tol, y±tol]
    for (std::size_t i = 0; i < epCount; ++i)
    {
      int cid = static_cast<int>(i);
      // Re-extract point position
      int curveIdx = static_cast<int>(i / 2);
      int endIdx   = static_cast<int>(i % 2);
      const gp_Pnt2d p = getEndpoint(EndpointRef{curveIdx, endIdx});
      const double xmin = p.X() - tol, xmax = p.X() + tol;
      const double ymin = p.Y() - tol, ymax = p.Y() + tol;
      kdt.rangeQuery(xmin, ymin, xmax, ymax, [&](const KDPoint& q){
        if (q.id > cid) // avoid duplicate unions
        {
          // Match original samePoint semantics (axis-wise tolerance)
          if (nearlyEqual(p.X(), q.x, tol) && nearlyEqual(p.Y(), q.y, tol))
          {
            ufUnion(static_cast<std::size_t>(cid), static_cast<std::size_t>(q.id));
          }
        }
      });
    }
  }

  // Apply explicit coincident constraints
  for (const auto& c : constraints_)
  {
    if (c.type != ConstraintType::Coincident)
      continue;
    ufUnion(endpointKey(c.a), endpointKey(c.b));
  }

  // Compute cluster representatives -> average positions of each cluster
  struct Acc
  {
    double x{0}, y{0};
    std::size_t n{0};
  };
  std::unordered_map<std::size_t, Acc> accum;
  for (std::size_t i = 0; i < curves_.size(); ++i)
  {
    for (int e = 0; e < 2; ++e)
    {
      EndpointRef r{static_cast<int>(i), e};
      const auto rep = ufFind(endpointKey(r));
      const auto p = getEndpoint(r);
      auto& a = accum[rep];
      a.x += p.X();
      a.y += p.Y();
      a.n += 1;
    }
  }

  // Assign averaged positions to each endpoint cluster
  std::unordered_map<std::size_t, gp_Pnt2d> clusterPoint;
  clusterPoint.reserve(accum.size());
  for (const auto& kv : accum)
  {
    const auto rep = kv.first;
    const auto& a = kv.second;
    clusterPoint.emplace(rep, gp_Pnt2d(a.x / static_cast<double>(a.n), a.y / static_cast<double>(a.n)));
  }

  // Write back averaged positions to all endpoints according to their cluster
  for (std::size_t i = 0; i < curves_.size(); ++i)
  {
    for (int e = 0; e < 2; ++e)
    {
      EndpointRef r{static_cast<int>(i), e};
      const auto rep = ufFind(endpointKey(r));
      setEndpoint(r, clusterPoint.at(rep));
    }
  }
}

std::vector<Sketch::Wire> Sketch::computeWires(double tol) const
{
  // Build adjacency graph among curves via endpoint coincidence
  const std::size_t n = curves_.size();
  std::vector<std::vector<int>> adj(n);
  // Map from representative endpoint cluster -> curves touching it
  std::unordered_map<std::size_t, std::vector<int>> clusterCurves;

  // Initialize UF if empty (e.g., computeWires called before solveConstraints)
  if (uf_parent_.size() != n * 2)
  {
    // build a temporary UF based on geometry-only clustering via k-d tree
    const_cast<Sketch*>(this)->ufInit(n * 2);
    if (n > 0)
    {
      const std::size_t epCount = n * 2;
      std::vector<KDPoint> pts;
      pts.reserve(epCount);
      for (std::size_t i = 0; i < n; ++i)
      {
        for (int e = 0; e < 2; ++e)
        {
          EndpointRef r{static_cast<int>(i), e};
          const auto p = getEndpoint(r);
          pts.push_back(KDPoint{p.X(), p.Y(), static_cast<int>(endpointKey(r))});
        }
      }
      KDTree2D kdt(std::move(pts));
      for (std::size_t i = 0; i < epCount; ++i)
      {
        int cid = static_cast<int>(i);
        int curveIdx = static_cast<int>(i / 2);
        int endIdx   = static_cast<int>(i % 2);
        const gp_Pnt2d p = getEndpoint(EndpointRef{curveIdx, endIdx});
        const double xmin = p.X() - tol, xmax = p.X() + tol;
        const double ymin = p.Y() - tol, ymax = p.Y() + tol;
        kdt.rangeQuery(xmin, ymin, xmax, ymax, [&](const KDPoint& q){
          if (q.id > cid)
          {
            if (nearlyEqual(p.X(), q.x, tol) && nearlyEqual(p.Y(), q.y, tol))
            {
              const_cast<Sketch*>(this)->ufUnion(static_cast<std::size_t>(cid), static_cast<std::size_t>(q.id));
            }
          }
        });
      }
    }
  }

  for (int cid = 0; cid < static_cast<int>(n); ++cid)
  {
    for (int e = 0; e < 2; ++e)
    {
      EndpointRef r{cid, e};
      const auto rep = ufFind(endpointKey(r));
      clusterCurves[rep].push_back(cid);
    }
  }

  // Connect curves that share an endpoint cluster
  for (const auto& kv : clusterCurves)
  {
    const auto& cs = kv.second;
    for (std::size_t i = 0; i < cs.size(); ++i)
    {
      for (std::size_t j = i + 1; j < cs.size(); ++j)
      {
        adj[cs[i]].push_back(cs[j]);
        adj[cs[j]].push_back(cs[i]);
      }
    }
  }

  // BFS to find connected components as wires
  std::vector<char> visited(n, 0);
  std::vector<Wire> wires;
  for (int i = 0; i < static_cast<int>(n); ++i)
  {
    if (visited[i])
      continue;
    // Isolated curve still forms a single-wire component
    Wire w;
    std::deque<int> dq;
    dq.push_back(i);
    visited[i] = 1;
    while (!dq.empty())
    {
      int u = dq.front();
      dq.pop_front();
      w.curves.push_back(u);
      for (int v : adj[u])
      {
        if (!visited[v])
        {
          visited[v] = 1;
          dq.push_back(v);
        }
      }
    }
    wires.push_back(std::move(w));
  }
  return wires;
}

std::vector<Sketch::OrderedPath> Sketch::computeOrderedPaths(double tol) const
{
  // Build mapping: cluster -> list of (curveId, endIndex)
  const std::size_t n = curves_.size();
  std::unordered_map<std::size_t, std::vector<std::pair<int, int>>> clusterRefs;

  // Ensure union-find reflects current coincidences
  if (uf_parent_.size() != n * 2)
  {
    const_cast<Sketch*>(this)->ufInit(n * 2);
  }

  for (int cid = 0; cid < static_cast<int>(n); ++cid)
  {
    for (int e = 0; e < 2; ++e)
    {
      EndpointRef r{cid, e};
      const auto rep = ufFind(endpointKey(r));
      clusterRefs[rep].push_back({cid, e});
    }
  }

  // Build wires first (connected sets of curves)
  const auto wires = computeWires(tol);

  // For each wire, create one or more ordered paths that cover all its curves
  std::vector<OrderedPath> paths;
  std::vector<char> used(static_cast<std::size_t>(n), 0);

  // Utility to find cluster representative for a curve endpoint
  auto epCluster = [&](int curveId, int endIndex) -> std::size_t {
    return ufFind(endpointKey(EndpointRef{curveId, endIndex}));
  };

  for (const auto& w : wires)
  {
    // Mark all curves in this component as unused
    for (int cid : w.curves)
      used[static_cast<std::size_t>(cid)] = 0;

    // Remaining degree per cluster considering only unused curves
    std::unordered_map<std::size_t, int> degree;
    for (int cid : w.curves)
    {
      degree[epCluster(cid, 0)]++;
      degree[epCluster(cid, 1)]++;
    }

    auto pickStartCluster = [&]() -> std::size_t {
      // Prefer odd-degree cluster for Eulerian path
      for (const auto& kv : degree)
      {
        if (kv.second % 2 == 1)
          return kv.first;
      }
      // Otherwise take any cluster in this component
      return epCluster(w.curves.front(), 0);
    };

    // While there are unused curves in this component, build a path
    std::size_t remaining = w.curves.size();
    for (int cid : w.curves)
      if (used[static_cast<std::size_t>(cid)])
        --remaining;

    while (true)
    {
      int notUsedCount = 0;
      for (int cid : w.curves)
        if (!used[static_cast<std::size_t>(cid)])
          ++notUsedCount;
      if (notUsedCount == 0)
        break;

      OrderedPath path;
      std::size_t startCluster = pickStartCluster();
      std::size_t currentCluster = startCluster;

      // Greedy walk: always take the smallest-id unused curve incident to current cluster
      while (true)
      {
        int nextCurve = -1;
        int nextEndIndex = 0;
        // Iterate candidates at the current cluster
        auto it = clusterRefs.find(currentCluster);
        if (it == clusterRefs.end())
          break;
        for (const auto& ref : it->second)
        {
          int cid = ref.first;
          if (used[static_cast<std::size_t>(cid)])
            continue;
          // Ensure this curve belongs to the current wire
          if (std::find(w.curves.begin(), w.curves.end(), cid) == w.curves.end())
            continue;
          // Prefer the lowest id for determinism
          if (nextCurve == -1 || cid < nextCurve)
          {
            nextCurve = cid;
            nextEndIndex = ref.second;  // which endpoint touches currentCluster
          }
        }

        if (nextCurve == -1)
          break;  // dead-end

        used[static_cast<std::size_t>(nextCurve)] = 1;

        // Orientation: if we are at endIndex 0, curve goes forward; if 1, reverse
        bool reversed = (nextEndIndex == 1);
        path.push_back(OrderedCurve{nextCurve, reversed});

        // Advance to the other endpoint's cluster
        int otherEnd = (nextEndIndex == 0) ? 1 : 0;
        currentCluster = epCluster(nextCurve, otherEnd);

        // Decrease degrees to bias next start toward remaining odd-degree clusters
        degree[currentCluster]--;
      }

      if (!path.empty())
        paths.push_back(std::move(path));
      else
        break;
    }
  }

  return paths;
}

std::vector<TopoDS_Wire> Sketch::toOcctWires(double tol) const
{
  auto paths = computeOrderedPaths(tol);
  std::vector<TopoDS_Wire> wires;
  wires.reserve(paths.size());

  // Map 2D sketch coordinates (u,v) into 3D by Ax2: P = O + u*X + v*Y
  const gp_Pnt  O  = m_ax2.Location();
  const gp_Dir  Xd = m_ax2.XDirection();
  const gp_Dir  Yd = m_ax2.YDirection();

  auto toPnt = [&](const gp_Pnt2d& p) {
    const double u = p.X();
    const double v = p.Y();
    gp_Vec vx(Xd.XYZ()); vx.Multiply(u);
    gp_Vec vy(Yd.XYZ()); vy.Multiply(v);
    return O.Translated(vx + vy);
  };
  const double PI2 = 2.0 * M_PI;

  // Angle of vector CP in the sketch's local X/Y basis
  auto angleOf = [&](const gp_Pnt2d& c, const gp_Pnt2d& p) {
    const double ux = p.X() - c.X();
    const double uy = p.Y() - c.Y();
    return std::atan2(uy, ux);
  };

  for (const auto& path : paths)
  {
    BRepBuilderAPI_MakeWire mw;
    for (const auto& oc : path)
    {
      const auto& c = curves_.at(static_cast<std::size_t>(oc.id));
      if (c.type == CurveType::Line)
      {
        gp_Pnt2d a = oc.reversed ? c.line.p2 : c.line.p1;
        gp_Pnt2d b = oc.reversed ? c.line.p1 : c.line.p2;
        // Skip degenerate edges (zero or near-zero length)
        const double dx = a.X() - b.X();
        const double dy = a.Y() - b.Y();
        if (dx*dx + dy*dy <= tol*tol)
          continue;
        BRepBuilderAPI_MakeEdge me(toPnt(a), toPnt(b));
        mw.Add(me.Edge());
      }
      else
      {
        // Build 3D circle in sketch plane (Ax2)
        const gp_Pnt2d& center = c.arc.center;
        // Circle center in 3D by mapping (cx,cy)
        gp_Pnt c3 = toPnt(center);
        gp_Ax2 ax2(c3, m_ax2.Direction(), m_ax2.XDirection());
        double r = center.Distance(c.arc.p1);
        gp_Circ circ(ax2, r);

        gp_Pnt2d a2d = oc.reversed ? c.arc.p2 : c.arc.p1;
        gp_Pnt2d b2d = oc.reversed ? c.arc.p1 : c.arc.p2;

        double u1 = angleOf(center, a2d);
        double u2 = angleOf(center, b2d);

        // Determine orientation along path: we choose the shorter arc that follows increasing parameter
        // Ensure u2 is ahead of u1 for forward sweep
        if (u2 < u1)
          u2 += PI2;

        // Skip degenerate arcs (no sweep or near-zero radius)
        if (r <= tol || std::abs(u2 - u1) <= 1.0e-12)
          continue;

        Handle(Geom_Circle) gc = new Geom_Circle(circ);
        Handle(Geom_TrimmedCurve) arc = new Geom_TrimmedCurve(gc, u1, u2);
        BRepBuilderAPI_MakeEdge me(arc);
        mw.Add(me.Edge());
      }
    }
    TopoDS_Wire w = mw.Wire();
    if (!w.IsNull())
      wires.push_back(w);
  }
  return wires;
}

// Serialization format (line-based):
// curves N
//  L x1 y1 x2 y2
//  A cx cy x1 y1 x2 y2 cw(0|1)
// constraints M
//  C aCurve aEnd bCurve bEnd
// plane ox oy oz zx zy zz xx xy xz
// planeId <id>        (optional)
std::string Sketch::serialize() const
{
  std::ostringstream os;
  os << "curves " << curves_.size() << "\n";
  for (const auto& c : curves_)
  {
    if (c.type == CurveType::Line)
    {
      os << "L " << c.line.p1.X() << ' ' << c.line.p1.Y() << ' '
         << c.line.p2.X() << ' ' << c.line.p2.Y() << "\n";
    }
    else
    {
      os << "A " << c.arc.center.X() << ' ' << c.arc.center.Y() << ' '
         << c.arc.p1.X() << ' ' << c.arc.p1.Y() << ' '
         << c.arc.p2.X() << ' ' << c.arc.p2.Y() << ' '
         << (c.arc.clockwise ? 1 : 0) << "\n";
    }
  }
  // Points block
  os << "points " << points_.size() << "\n";
  for (const auto& p : points_)
  {
    os << "P " << p.X() << ' ' << p.Y() << "\n";
  }
  os << "constraints " << constraints_.size() << "\n";
  for (const auto& k : constraints_)
  {
    if (k.type == ConstraintType::Coincident)
    {
      os << "C " << k.a.curve << ' ' << k.a.endIndex << ' ' << k.b.curve << ' ' << k.b.endIndex << "\n";
    }
  }
  // Plane binding
  const gp_Pnt loc = m_ax2.Location();
  const gp_Dir Z   = m_ax2.Direction();
  const gp_Dir X   = m_ax2.XDirection();
  os << "plane "
     << loc.X() << ' ' << loc.Y() << ' ' << loc.Z() << ' '
     << Z.X()   << ' ' << Z.Y()   << ' ' << Z.Z()   << ' '
     << X.X()   << ' ' << X.Y()   << ' ' << X.Z()   << "\n";
  if (m_planeId != 0)
  {
    os << "planeId " << m_planeId << "\n";
  }
  return os.str();
}

void Sketch::deserialize(const std::string& data)
{
  curves_.clear();
  constraints_.clear();
  points_.clear();
  // Defaults: XY plane at origin
  m_ax2 = gp_Ax2(gp_Pnt(0,0,0), gp::DZ(), gp::DX());
  m_planeId = 0;
  std::istringstream is(data);
  std::string head;
  std::size_t n = 0;
  if (is >> head >> n && head == "curves")
  {
    for (std::size_t i = 0; i < n; ++i)
    {
      char typ; is >> typ;
      if (typ == 'L')
      {
        double x1,y1,x2,y2; is >> x1 >> y1 >> x2 >> y2;
        addLine(gp_Pnt2d(x1,y1), gp_Pnt2d(x2,y2));
      }
      else if (typ == 'A')
      {
        double cx,cy,x1,y1,x2,y2; int cw; is >> cx >> cy >> x1 >> y1 >> x2 >> y2 >> cw;
        addArc(gp_Pnt2d(cx,cy), gp_Pnt2d(x1,y1), gp_Pnt2d(x2,y2), cw != 0);
      }
    }
  }
  // Optionally points block may follow directly after curves
  if (is >> head >> n && head == "points")
  {
    for (std::size_t i = 0; i < n; ++i)
    {
      char typ; is >> typ;
      if (typ == 'P')
      {
        double x,y; is >> x >> y;
        addPoint(gp_Pnt2d(x,y));
      }
    }
  }

  if (is >> head >> n && head == "constraints")
  {
    for (std::size_t i = 0; i < n; ++i)
    {
      char typ; is >> typ;
      if (typ == 'C')
      {
        int ac, ae, bc, be; is >> ac >> ae >> bc >> be;
        addCoincident(EndpointRef{ac, ae}, EndpointRef{bc, be});
      }
    }
  }

  // Optional trailing sections (order independent after the two blocks)
  // Parse until EOF: recognize "plane" and "planeId" tokens
  while (is >> head)
  {
    if (head == "plane")
    {
      double ox,oy,oz, zx,zy,zz, xx,xy,xz;
      if (is >> ox >> oy >> oz >> zx >> zy >> zz >> xx >> xy >> xz)
      {
        gp_Pnt o(ox,oy,oz);
        gp_Dir Z(zx,zy,zz);
        gp_Dir X(xx,xy,xz);
        m_ax2 = gp_Ax2(o, Z, X);
      }
    }
    else if (head == "planeId")
    {
      unsigned long long pid = 0ULL;
      if (is >> pid)
      {
        m_planeId = static_cast<DocumentItem::Id>(pid);
      }
    }
    else
    {
      // Unknown token: skip rest of line
      is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
  }
}


gp_Pnt2d Sketch::getEndpoint(const EndpointRef& r) const
{
  const auto& c = curves_.at(static_cast<std::size_t>(r.curve));
  if (c.type == CurveType::Line)
  {
    return (r.endIndex == 0) ? c.line.p1 : c.line.p2;
  }
  else
  {
    return (r.endIndex == 0) ? c.arc.p1 : c.arc.p2;
  }
}

void Sketch::setEndpoint(const EndpointRef& r, const gp_Pnt2d& p)
{
  auto& c = curves_.at(static_cast<std::size_t>(r.curve));
  if (c.type == CurveType::Line)
  {
    if (r.endIndex == 0)
      c.line.p1 = p;
    else
      c.line.p2 = p;
  }
  else
  {
    if (r.endIndex == 0)
      c.arc.p1 = p;
    else
      c.arc.p2 = p;
  }
}

void Sketch::ufInit(std::size_t n)
{
  uf_parent_.resize(n);
  for (std::size_t i = 0; i < n; ++i)
    uf_parent_[i] = i;
}

std::size_t Sketch::ufFind(std::size_t i) const
{
  if (uf_parent_[i] == i)
    return i;
  // Path compression: mutable parent vector
  auto root = ufFind(uf_parent_[i]);
  const_cast<std::vector<std::size_t>&>(uf_parent_)[i] = root;
  return root;
}

void Sketch::ufUnion(std::size_t a, std::size_t b)
{
  a = ufFind(a);
  b = ufFind(b);
  if (a == b)
    return;
  uf_parent_[b] = a;
}
