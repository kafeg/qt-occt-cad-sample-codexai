#include "Document.h"
#include <ExtrudeFeature.h>
#include <Sketch.h>

void Document::clear()
{
  m_features.Clear();
  m_items.clear();
}

void Document::addFeature(const Handle(Feature)& f)
{
  m_features.Append(f);
}

void Document::recompute()
{
  for (NCollection_Sequence<Handle(Feature)>::Iterator it(m_features); it.More(); it.Next()) {
    const Handle(Feature)& f = it.Value();
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
      f->execute();
    }
  }
}

void Document::removeLast()
{
  if (!m_features.IsEmpty())
  {
    m_features.Remove(m_features.Size());
  }
}

void Document::removeFeature(const Handle(Feature)& f)
{
  if (f.IsNull()) return;
  for (int i = 1; i <= m_features.Size(); ++i)
  {
    if (m_features.Value(i) == f)
    {
      m_features.Remove(i);
      break;
    }
  }
}

void Document::addItem(const std::shared_ptr<DocumentItem>& item)
{
  if (!item) return;
  m_items[item->id()] = item;
}

void Document::addSketch(const std::shared_ptr<Sketch>& s)
{
  addItem(s);
}

std::shared_ptr<Sketch> Document::findSketch(DocumentItem::Id id) const
{
  auto it = m_items.find(id);
  if (it == m_items.end()) return {};
  return std::dynamic_pointer_cast<Sketch>(it->second);
}

std::vector<std::shared_ptr<Sketch>> Document::sketches() const
{
  std::vector<std::shared_ptr<Sketch>> out;
  out.reserve(m_items.size());
  for (const auto& kv : m_items)
  {
    if (auto sk = std::dynamic_pointer_cast<Sketch>(kv.second))
      out.push_back(std::move(sk));
  }
  return out;
}
