#include <gtest/gtest.h>

#include <BRepPrimAPI_MakeBox.hxx>
#include <TopExp_Explorer.hxx>
#include <TopAbs_ShapeEnum.hxx>

TEST(OpenCASCADE, MakeBoxHasFaces) {
  BRepPrimAPI_MakeBox mkBox(10.0, 20.0, 30.0);
  TopoDS_Shape shape = mkBox.Shape();
  int faceCount = 0;
  for (TopExp_Explorer exp(shape, TopAbs_FACE); exp.More(); exp.Next()) {
    ++faceCount;
  }
  EXPECT_EQ(faceCount, 6);
}

