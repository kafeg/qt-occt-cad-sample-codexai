#include "Document.h"

void Document::clear()
{
  m_features.Clear();
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
