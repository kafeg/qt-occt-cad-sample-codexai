#pragma once

#include <Standard_DefineHandle.hxx>
#include <Standard_Transient.hxx>
#include <TopoDS_Shape.hxx>
#include <TCollection_AsciiString.hxx>

#include <unordered_map>
#include <variant>
#include <string>

class Feature;
DEFINE_STANDARD_HANDLE(Feature, Standard_Transient)

// Abstract feature with parameters and resulting shape
class Feature : public Standard_Transient
{
  DEFINE_STANDARD_RTTIEXT(Feature, Standard_Transient)

public:
  // Common parameter keys across all features
  enum class ParamKey
  {
    Dx,
    Dy,
    Dz,
    Radius,
    Height,
  };

  struct ParamKeyHash
  {
    std::size_t operator()(ParamKey k) const noexcept { return static_cast<std::size_t>(k); }
  };

  using ParamValue = std::variant<int, double, TCollection_AsciiString>;
  using ParamMap   = std::unordered_map<ParamKey, ParamValue, ParamKeyHash>;

  virtual ~Feature() = default;

  // Compute the resulting shape using current parameters
  virtual void execute() = 0;

  // Access computed shape
  virtual const TopoDS_Shape& shape() const { return m_shape; }

  // Optional: basic name and parameter accessors
  const TCollection_AsciiString& name() const { return m_name; }

  void setName(const TCollection_AsciiString& theName) { m_name = theName; }

  const ParamMap& params() const { return m_params; }

  ParamMap& params() { return m_params; }

protected:
  TCollection_AsciiString m_name;
  ParamMap                m_params;
  TopoDS_Shape            m_shape; // resulting shape
};
