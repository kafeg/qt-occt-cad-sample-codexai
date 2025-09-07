#include "Document.h"
#include <ExtrudeFeature.h>
#include <MoveFeature.h>
#include <Sketch.h>
#include <PlaneFeature.h>
#include <PointFeature.h>
#include <Datum.h>
#include <AxeFeature.h>
#include "DocumentInitializer.h"
#include <algorithm>

Document::Document()
{
  // Initialize a default Datum at creation time
  m_datum = std::make_shared<Datum>();
  // Populate default geometry (planes and optional origin point)
  DocumentInitializer::initialize(*this);
}

void Document::clear()
{
  m_items.Clear();
  m_registry.clear();
  m_sketchList.clear();
  m_featuresCache.Clear();
  m_featuresCacheDirty = true;
  // Keep Datum persistent; recreate default if missing
  if (!m_datum) m_datum = std::make_shared<Datum>();
  // Clear planes container
  m_planes.Clear();
  // Recreate default geometry using current Datum settings
  DocumentInitializer::initialize(*this);
}

void Document::addItem(const Handle(DocumentItem)& item)
{
  if (!item.IsNull())
  {
    m_items.Append(item);
    m_featuresCacheDirty = true;
    if (Handle(PlaneFeature) pf = Handle(PlaneFeature)::DownCast(item); !pf.IsNull())
    {
      m_planes.Append(pf);
    }
  }
}

void Document::insertItem(int index1, const Handle(DocumentItem)& item)
{
  if (item.IsNull()) return;
  if (index1 < 1) index1 = 1;
  if (index1 > m_items.Size() + 1) index1 = m_items.Size() + 1;
  m_items.InsertBefore(index1, item);
  m_featuresCacheDirty = true;
  if (Handle(PlaneFeature) pf = Handle(PlaneFeature)::DownCast(item); !pf.IsNull())
  {
    m_planes.Append(pf);
  }
}

const NCollection_Sequence<Handle(Feature)>& Document::features() const
{
  if (m_featuresCacheDirty)
  {
    m_featuresCache.Clear();
    for (NCollection_Sequence<Handle(DocumentItem)>::Iterator it(m_items); it.More(); it.Next())
    {
      const Handle(DocumentItem)& di = it.Value();
      if (Handle(Feature) f = Handle(Feature)::DownCast(di); !f.IsNull()) {
        // Exclude fixed-geometry helpers (planes, axes and origin point) from generic features
        if (Handle(PlaneFeature)::DownCast(f).IsNull() && Handle(PointFeature)::DownCast(f).IsNull() && Handle(AxeFeature)::DownCast(f).IsNull())
          m_featuresCache.Append(f);
      }
    }
    m_featuresCacheDirty = false;
  }
  return m_featuresCache;
}

void Document::recompute()
{
  // Map already-executed features by id for downstream dependency resolution
  std::unordered_map<DocumentItem::Id, Handle(Feature)> featureById;
  for (NCollection_Sequence<Handle(DocumentItem)>::Iterator it(m_items); it.More(); it.Next()) {
    const Handle(DocumentItem)& di = it.Value();
    Handle(Feature) f = Handle(Feature)::DownCast(di);
    if (f.IsNull()) continue;
    if (!f.IsNull() && !f->isSuppressed()) {
      // Resolve dependencies for known feature types
      if (Handle(ExtrudeFeature) ef = Handle(ExtrudeFeature)::DownCast(f); !ef.IsNull())
      {
        if (!ef->sketch() && ef->sketchId() != 0)
        {
          if (auto sk = findSketch(ef->sketchId()))
          {
            ef->setSketch(sk);
          }
        }
      }
      // Resolve MoveFeature source by id; allow using suppressed sources as providers
      if (Handle(MoveFeature) mf = Handle(MoveFeature)::DownCast(f); !mf.IsNull())
      {
        if (mf->source().IsNull() && mf->sourceId() != 0)
        {
          Handle(Feature) src;
          auto fit = featureById.find(mf->sourceId());
          if (fit != featureById.end())
          {
            src = fit->second;
          }
          if (src.IsNull())
          {
            // search previous items for a feature with the same id (even if suppressed)
            for (NCollection_Sequence<Handle(DocumentItem)>::Iterator jt(m_items); jt.More(); jt.Next())
            {
              const Handle(DocumentItem)& prev = jt.Value();
              if (prev == di) break; // stop at current
              Handle(Feature) pf = Handle(Feature)::DownCast(prev);
              if (!pf.IsNull() && pf->id() == mf->sourceId()) { src = pf; break; }
            }
          }
          if (!src.IsNull())
          {
            // ensure source has valid shape, even if suppressed
            if (src->shape().IsNull()) { src->execute(); }
            mf->setSource(src);
          }
        }
      }
      f->execute();
      // cache executed (non-suppressed and also MoveFeature) for downstream consumers
      featureById[f->id()] = f;
    }
  }
}

void Document::removeLast()
{
  if (!m_items.IsEmpty())
  {
    m_items.Remove(m_items.Size());
    m_featuresCacheDirty = true;
  }
}

void Document::removeFeature(const Handle(Feature)& f)
{
  if (f.IsNull()) return;
  for (int i = 1; i <= m_items.Size(); ++i)
  {
    Handle(Feature) fi = Handle(Feature)::DownCast(m_items.Value(i));
    if (!fi.IsNull() && fi == f)
    {
      m_items.Remove(i);
      m_featuresCacheDirty = true;
      // If it is a plane, remove from planes container as well
      if (!Handle(PlaneFeature)::DownCast(f).IsNull())
      {
        for (NCollection_Sequence<Handle(PlaneFeature)>::Iterator pit(m_planes); pit.More(); pit.Next())
        {
          if (pit.Value()->id() == f->id()) { m_planes.Remove(pit); break; }
        }
      }
      break;
    }
  }
}

void Document::removeItem(const Handle(DocumentItem)& it)
{
  if (it.IsNull()) return;
  for (int i = 1; i <= m_items.Size(); ++i)
  {
    if (m_items.Value(i) == it)
    {
      m_items.Remove(i);
      m_featuresCacheDirty = true;
      break;
    }
  }
}

void Document::removeSketchById(DocumentItem::Id id)
{
  if (id == 0) return;
  // Erase from registry
  m_registry.erase(id);
  // Erase from ordered sketch list
  m_sketchList.erase(std::remove_if(m_sketchList.begin(), m_sketchList.end(), [id](const std::shared_ptr<Sketch>& s){ return s && s->id() == id; }), m_sketchList.end());
}

void Document::addItem(const std::shared_ptr<DocumentItem>& item)
{
  if (!item) return;
  m_registry[item->id()] = item;
}

void Document::addSketch(const std::shared_ptr<Sketch>& s)
{
  if (!s) return;
  const auto sid = s->id();
  // Register in the generic item registry for dependency resolution (overwrites by id)
  m_registry[sid] = s;
  // Preserve insertion order for sketches; avoid duplicates by id
  const bool exists = std::any_of(m_sketchList.begin(), m_sketchList.end(), [sid](const std::shared_ptr<Sketch>& p){ return p && p->id() == sid; });
  if (!exists)
  {
    m_sketchList.push_back(s);
  }

  // Ensure a corresponding timeline entry exists in items() so history reflects sketches
  bool inTimeline = false;
  for (NCollection_Sequence<Handle(DocumentItem)>::Iterator it(m_items); it.More(); it.Next())
  {
    const Handle(DocumentItem)& di = it.Value();
    if (!di.IsNull() && di->kind() == DocumentItem::Kind::Sketch && di->id() == sid)
    {
      inTimeline = true;
      break;
    }
  }
  if (!inTimeline)
  {
    Handle(Sketch) hs = new Sketch(sid); // lightweight handle mirror with same id
    // No need to serialize full geometry for history labeling; id is sufficient
    addItem(Handle(DocumentItem)(hs));
  }
}

std::shared_ptr<Sketch> Document::findSketch(DocumentItem::Id id) const
{
  auto it = m_registry.find(id);
  if (it == m_registry.end()) return {};
  return std::dynamic_pointer_cast<Sketch>(it->second);
}

std::vector<std::shared_ptr<Sketch>> Document::sketches() const
{
  // Return the ordered list to keep stable, insertion-order enumeration
  return m_sketchList;
}

void Document::addPlane(const Handle(PlaneFeature)& p)
{
  if (p.IsNull()) return;
  addItem(Handle(DocumentItem)(p));
}

Handle(PlaneFeature) Document::findPlane(DocumentItem::Id id) const
{
  for (NCollection_Sequence<Handle(PlaneFeature)>::Iterator it(m_planes); it.More(); it.Next())
  {
    const Handle(PlaneFeature)& pf = it.Value();
    if (!pf.IsNull() && pf->id() == id) return pf;
  }
  // fallback: search timeline
  for (NCollection_Sequence<Handle(DocumentItem)>::Iterator it(m_items); it.More(); it.Next())
  {
    Handle(PlaneFeature) pf = Handle(PlaneFeature)::DownCast(it.Value());
    if (!pf.IsNull() && pf->id() == id) return pf;
  }
  return Handle(PlaneFeature)();
}
