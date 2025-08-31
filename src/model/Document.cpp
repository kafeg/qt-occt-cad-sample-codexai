#include "Document.h"

void Document::clear()
{
  myFeatures.Clear();
}

void Document::addFeature(const Feature& f)
{
  myFeatures.Append(f);
}

