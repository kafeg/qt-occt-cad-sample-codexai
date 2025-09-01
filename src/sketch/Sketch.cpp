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

Sketch::CurveId Sketch::addArc(const gp_Pnt2d& center, const gp_Pnt2d& a, const gp_Pnt2d& b, bool clockwise)
{
  Curve c;
  c.type = CurveType::Arc;
  c.arc = Arc{center, a, b, clockwise};
  curves_.push_back(c);
  return static_cast<CurveId>(curves_.size() - 1);
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

  // First, union existing near-coincident endpoints (geometric proximity)
  for (std::size_t i = 0; i < curves_.size(); ++i)
  {
    for (std::size_t j = i + 1; j < curves_.size(); ++j)
    {
      for (int ei = 0; ei < 2; ++ei)
      {
        EndpointRef ri{static_cast<int>(i), ei};
        const auto pi = getEndpoint(ri);
        for (int ej = 0; ej < 2; ++ej)
        {
          EndpointRef rj{static_cast<int>(j), ej};
          const auto pj = getEndpoint(rj);
          if (samePoint(pi, pj, tol))
          {
            ufUnion(endpointKey(ri), endpointKey(rj));
          }
        }
      }
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
    // build a temporary UF based on geometry-only clustering
    const_cast<Sketch*>(this)->ufInit(n * 2);
    for (std::size_t i = 0; i < n; ++i)
    {
      for (std::size_t j = i + 1; j < n; ++j)
      {
        for (int ei = 0; ei < 2; ++ei)
        {
          EndpointRef ri{static_cast<int>(i), ei};
          const auto pi = getEndpoint(ri);
          for (int ej = 0; ej < 2; ++ej)
          {
            EndpointRef rj{static_cast<int>(j), ej};
            const auto pj = getEndpoint(rj);
            if (samePoint(pi, pj, tol))
            {
              const_cast<Sketch*>(this)->ufUnion(endpointKey(ri), endpointKey(rj));
            }
          }
        }
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

  auto toPnt = [](const gp_Pnt2d& p) { return gp_Pnt(p.X(), p.Y(), 0.0); };
  const double PI2 = 2.0 * M_PI;

  auto angleOf = [](const gp_Pnt2d& c, const gp_Pnt2d& p) {
    return std::atan2(p.Y() - c.Y(), p.X() - c.X());
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
        BRepBuilderAPI_MakeEdge me(toPnt(a), toPnt(b));
        mw.Add(me.Edge());
      }
      else
      {
        // Build 3D circle in XY plane
        const gp_Pnt2d& center = c.arc.center;
        gp_Ax2 ax2(gp_Pnt(center.X(), center.Y(), 0.0), gp::DZ());
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

        Handle(Geom_Circle) gc = new Geom_Circle(circ);
        Handle(Geom_TrimmedCurve) arc = new Geom_TrimmedCurve(gc, u1, u2);
        BRepBuilderAPI_MakeEdge me(arc);
        mw.Add(me.Edge());
      }
    }
    wires.push_back(mw.Wire());
  }
  return wires;
}

// Serialization format (line-based):
// curves N
//  L x1 y1 x2 y2
//  A cx cy x1 y1 x2 y2 cw(0|1)
// constraints M
//  C aCurve aEnd bCurve bEnd
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
  os << "constraints " << constraints_.size() << "\n";
  for (const auto& k : constraints_)
  {
    if (k.type == ConstraintType::Coincident)
    {
      os << "C " << k.a.curve << ' ' << k.a.endIndex << ' ' << k.b.curve << ' ' << k.b.endIndex << "\n";
    }
  }
  return os.str();
}

void Sketch::deserialize(const std::string& data)
{
  curves_.clear();
  constraints_.clear();
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
