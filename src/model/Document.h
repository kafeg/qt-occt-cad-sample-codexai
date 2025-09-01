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
  void clear();                                               // Remove all items

  // Timeline manipulation (ordered history)
  void addItem(const Handle(DocumentItem)& item);             // Append an item
  void insertItem(int index1, const Handle(DocumentItem)& item); // Insert at 1-based index
  const NCollection_Sequence<Handle(DocumentItem)>& items() const { return m_items; }

  // Convenience helpers for features
  void addFeature(const Handle(Feature)& f) { addItem(Handle(DocumentItem)(f)); }
  const NCollection_Sequence<Handle(Feature)>& features() const; // Filtered view of items()
  void recompute();                                           // Execute features in order
  void removeLast();                                          // Pop last item
  void removeFeature(const Handle(Feature)& f);               // Remove by handle (first match)

  // Register and query DocumentItems (e.g., sketches) for dependency resolution
  void addItem(const std::shared_ptr<DocumentItem>& item);
  void addSketch(const std::shared_ptr<Sketch>& s);           // convenience wrapper
  std::shared_ptr<Sketch> findSketch(DocumentItem::Id id) const;
  std::vector<std::shared_ptr<Sketch>> sketches() const;      // list registered sketches

private:
  // Ordered document history (sketches, features, etc.)
  NCollection_Sequence<Handle(DocumentItem)> m_items;
  // Cached filtered view for features()
  mutable NCollection_Sequence<Handle(Feature)> m_featuresCache;
  mutable bool m_featuresCacheDirty{true};

  // Item registry for non-handle items used for dependency resolution (e.g., sketches held as std::shared_ptr)
  std::unordered_map<DocumentItem::Id, std::shared_ptr<DocumentItem>> m_registry;
};
