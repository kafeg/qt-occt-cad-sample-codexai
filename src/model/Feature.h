#pragma once

#include <NCollection_Handle.hxx>
#include <TopoDS_Shape.hxx>
#include <TCollection_AsciiString.hxx>

// Minimal feature representation
struct Feature
{
  TCollection_AsciiString name;
  TopoDS_Shape            shape; // resulting shape
};

