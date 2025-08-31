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
