#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include <Standard_DefineHandle.hxx>
#include <Standard_Transient.hxx>

// Base class for any item that can live in a Document and be serialized.
class DocumentItem : public Standard_Transient
{
public:
  using Id = std::uint64_t;

  enum class Kind
  {
    Sketch = 1,
    BoxFeature = 100,
    CylinderFeature = 101,
    ExtrudeFeature = 102,
    MoveFeature = 103,
  };

  virtual ~DocumentItem() = default;

  Id id() const { return m_id; }

  // Every item reports its kind for factory-driven reconstruction
  virtual Kind kind() const = 0;

  // Minimal serialization interface: encode to a string blob and restore from it
  virtual std::string serialize() const = 0;
  virtual void        deserialize(const std::string& data) = 0;

  // Factory registration and creation for document load
  using CreateFn = std::function<std::shared_ptr<DocumentItem>()>;

  static void registerFactory(Kind k, CreateFn fn);
  static std::shared_ptr<DocumentItem> create(Kind k);

protected:
  DocumentItem();
  explicit DocumentItem(Id existingId);

private:
  Id m_id{0};
};

// Enable OCCT handle for DocumentItem
DEFINE_STANDARD_HANDLE(DocumentItem, Standard_Transient)
