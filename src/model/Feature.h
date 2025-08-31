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
  using ParamValue = std::variant<int, double, TCollection_AsciiString>;
  using ParamMap   = std::unordered_map<std::string, ParamValue>;

  virtual ~Feature() = default;

  // Compute the resulting shape using current parameters
  virtual void execute() = 0;

  // Access computed shape
  virtual const TopoDS_Shape& shape() const { return myShape; }

  // Optional: basic name and parameter accessors
  const TCollection_AsciiString& name() const { return myName; }
  void setName(const TCollection_AsciiString& theName) { myName = theName; }

  const ParamMap& params() const { return myParams; }
  ParamMap& params() { return myParams; }

protected:
  TCollection_AsciiString myName;
  ParamMap                myParams;
  TopoDS_Shape           myShape; // resulting shape
};

