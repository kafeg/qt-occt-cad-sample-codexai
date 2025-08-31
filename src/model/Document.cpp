#include "Document.h"

void Document::clear()
{
  myFeatures.Clear();
}

void Document::addFeature(const Handle(Feature)& f)
{
  myFeatures.Append(f);
}

void Document::recompute()
{
  for (NCollection_Sequence<Handle(Feature)>::Iterator it(myFeatures); it.More(); it.Next()) {
    const Handle(Feature)& f = it.Value();
    if (!f.IsNull()) {
      f->execute();
    }
  }
}

void Document::removeLast()
{
  if (!myFeatures.IsEmpty())
  {
    myFeatures.Remove(myFeatures.Size());
  }
}

void Document::removeFeature(const Handle(Feature)& f)
{
  if (f.IsNull()) return;
  for (int i = 1; i <= myFeatures.Size(); ++i)
  {
    if (myFeatures.Value(i) == f)
    {
      myFeatures.Remove(i);
      break;
    }
  }
}
