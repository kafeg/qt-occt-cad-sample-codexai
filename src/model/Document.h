#pragma once

#include "Feature.h"
#include <NCollection_Sequence.hxx>

class Document
{
public:
  void clear();
  void addFeature(const Feature& f);
  const NCollection_Sequence<Feature>& features() const { return myFeatures; }

private:
  NCollection_Sequence<Feature> myFeatures;
};

