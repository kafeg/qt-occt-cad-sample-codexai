#pragma once

#include "Feature.h"
#include <NCollection_Sequence.hxx>

class Document
{
public:
  void clear();
  void addFeature(const Handle(Feature)& f);
  const NCollection_Sequence<Handle(Feature)>& features() const { return myFeatures; }
  void recompute();
  void removeLast();
  void removeFeature(const Handle(Feature)& f);

private:
  NCollection_Sequence<Handle(Feature)> myFeatures;
};
