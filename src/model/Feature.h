#pragma once

#include <Standard_DefineHandle.hxx>
#include <Standard_Transient.hxx>
#include <TopoDS_Shape.hxx>
#include <TCollection_AsciiString.hxx>

#include <unordered_map>
#include <variant>
#include <string>

#include <DocumentItem.h>

class Feature;
DEFINE_STANDARD_HANDLE(Feature, Standard_Transient)

// Abstract feature with typed parameter map and resulting shape
class Feature : public DocumentItem
{
  DEFINE_STANDARD_RTTIEXT(Feature, DocumentItem)

public:
  // Common parameter keys across all features
  enum class ParamKey // common keys shared by primitives
  {
    Dx,
    Dy,
    Dz,
    Radius,
    Height,
    Distance,
    Tx,
    Ty,
    Tz,
    Rx,
    Ry,
    Rz,
    // Plane params
    Ox,
    Oy,
    Oz,
    Nx,
    Ny,
    Nz,
    Size,
  };

  struct ParamKeyHash
  {
    std::size_t operator()(ParamKey k) const noexcept { return static_cast<std::size_t>(k); }
  };

  using ParamValue = std::variant<int, double, TCollection_AsciiString>;     // numeric or string param
  using ParamMap   = std::unordered_map<ParamKey, ParamValue, ParamKeyHash>; // keyed parameter map

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

  // Suppression flag: suppressed features are skipped during recompute and not displayed
  bool isSuppressed() const { return m_suppressed; }
  void setSuppressed(bool on) { m_suppressed = on; }

  // DocumentItem interface
  // Base Feature encodes common fields: name, suppressed flag, and params
  virtual Kind kind() const override = 0;
  std::string serialize() const override;
  void        deserialize(const std::string& data) override;

protected:
  TCollection_AsciiString m_name;
  ParamMap                m_params;
  TopoDS_Shape            m_shape; // resulting shape
  bool                    m_suppressed = false; // execution/display suppressed

  // Helper: read numeric parameter as double (accepts int/double; otherwise returns defVal)
  static double paramAsDouble(const ParamMap& pm, ParamKey key, double defVal);
};
