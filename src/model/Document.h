#pragma once

#include "Feature.h"
#include <NCollection_Sequence.hxx>

#include <DocumentItem.h>
#include <memory>
#include <unordered_map>

class Sketch;

// Minimal parametric document: ordered list of features and recompute
class Document
{
public:
  void clear();                                               // Remove all features
  void addFeature(const Handle(Feature)& f);                  // Append a feature
  const NCollection_Sequence<Handle(Feature)>& features() const { return m_features; } // Read-only access
  void recompute();                                           // Execute features in order
  void removeLast();                                          // Pop last feature
  void removeFeature(const Handle(Feature)& f);               // Remove by handle (first match)

  // Register and query DocumentItems (e.g., sketches) for dependency resolution
  void addItem(const std::shared_ptr<DocumentItem>& item);
  void addSketch(const std::shared_ptr<Sketch>& s);           // convenience wrapper
  std::shared_ptr<Sketch> findSketch(DocumentItem::Id id) const;
  std::vector<std::shared_ptr<Sketch>> sketches() const;      // list registered sketches

private:
  NCollection_Sequence<Handle(Feature)> m_features; // ordered model features
  std::unordered_map<DocumentItem::Id, std::shared_ptr<DocumentItem>> m_items; // item registry
};
