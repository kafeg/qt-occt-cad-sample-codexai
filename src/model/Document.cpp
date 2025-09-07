#include "Document.h"
#include <ExtrudeFeature.h>
#include <MoveFeature.h>
#include <Sketch.h>
#include <Datum.h>

Document::Document()
{
  // Initialize a default Datum at creation time
  m_datum = std::make_shared<Datum>();
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
}

void Document::addItem(const Handle(DocumentItem)& item)
{
  if (!item.IsNull())
  {
    m_items.Append(item);
    m_featuresCacheDirty = true;
  }
}

void Document::insertItem(int index1, const Handle(DocumentItem)& item)
{
  if (item.IsNull()) return;
  if (index1 < 1) index1 = 1;
  if (index1 > m_items.Size() + 1) index1 = m_items.Size() + 1;
  m_items.InsertBefore(index1, item);
  m_featuresCacheDirty = true;
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
      break;
    }
  }
}

void Document::addItem(const std::shared_ptr<DocumentItem>& item)
{
  if (!item) return;
  m_registry[item->id()] = item;
}

void Document::addSketch(const std::shared_ptr<Sketch>& s)
{
  if (!s) return;
  // Preserve insertion order for sketches
  m_sketchList.push_back(s);
  // Register in the generic item registry for dependency resolution
  m_registry[s->id()] = s;
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
