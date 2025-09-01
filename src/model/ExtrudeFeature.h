#pragma once

#include "Feature.h"
#include <Standard_DefineHandle.hxx>

#include <memory>
#include <DocumentItem.h>

class Sketch;

class ExtrudeFeature;
DEFINE_STANDARD_HANDLE(ExtrudeFeature, Feature)

// Extrude feature: references a 2D Sketch profile and a linear distance along +Z
class ExtrudeFeature : public Feature
{
  DEFINE_STANDARD_RTTIEXT(ExtrudeFeature, Feature)

public:
  ExtrudeFeature() = default;

  // ID-based constructor for upstream reference
  ExtrudeFeature(DocumentItem::Id sketchId, double distance)
    : m_sketchId(sketchId) { setDistance(distance); }

  void setSketch(const std::shared_ptr<Sketch>& sk) { m_sketch = sk; }
  const std::shared_ptr<Sketch>& sketch() const { return m_sketch; }

  // ID-based linkage for serialization-friendly dependency tracking
  void setSketchId(DocumentItem::Id id) { m_sketchId = id; }
  DocumentItem::Id sketchId() const { return m_sketchId; }

  void setDistance(double d) { params()[Feature::ParamKey::Distance] = d; }
  double distance() const;

  void execute() override;

private:
  std::shared_ptr<Sketch> m_sketch; // runtime profile (optional)
  DocumentItem::Id        m_sketchId{0}; // persistent reference

public:
  // DocumentItem
  Kind kind() const override { return Kind::ExtrudeFeature; }
  std::string serialize() const override;
  void        deserialize(const std::string& data) override;
};
