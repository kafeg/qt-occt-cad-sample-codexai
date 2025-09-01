#include <gtest/gtest.h>

#include <Sketch.h>
#include <BoxFeature.h>
#include <ExtrudeFeature.h>

TEST(Serialization, SketchRoundtrip)
{
  Sketch s;
  auto l1 = s.addLine(gp_Pnt2d(0,0), gp_Pnt2d(10,0));
  auto l2 = s.addLine(gp_Pnt2d(10,0), gp_Pnt2d(10,5));
  auto a1 = s.addArc(gp_Pnt2d(5,5), gp_Pnt2d(10,5), gp_Pnt2d(0,5), false);
  s.addCoincident({l1,1}, {l2,0});

  const std::string blob = s.serialize();

  Sketch s2;
  s2.deserialize(blob);

  ASSERT_EQ(s2.curves().size(), 3u);
  EXPECT_EQ(s2.curves()[0].type, Sketch::CurveType::Line);
  EXPECT_EQ(s2.curves()[1].type, Sketch::CurveType::Line);
  EXPECT_EQ(s2.curves()[2].type, Sketch::CurveType::Arc);

  s.solveConstraints();
  s2.solveConstraints();
  auto wires1 = s.computeWires();
  auto wires2 = s2.computeWires();
  ASSERT_EQ(wires1.size(), wires2.size());
  if (!wires1.empty())
    EXPECT_EQ(wires1[0].curves.size(), wires2[0].curves.size());
}

TEST(Serialization, FeatureBaseRoundtrip)
{
  Handle(BoxFeature) a = new BoxFeature();
  a->setName(TCollection_AsciiString("MyBox"));
  a->setSuppressed(true);
  a->setSize(3.0, 4.0, 5.0);

  const std::string blob = a->serialize();

  Handle(BoxFeature) b = new BoxFeature();
  b->deserialize(blob);

  EXPECT_STREQ(b->name().ToCString(), "MyBox");
  EXPECT_TRUE(b->isSuppressed());
  // Execute to validate params survived
  b->execute();
  ASSERT_FALSE(b->shape().IsNull());
}

TEST(Serialization, ExtrudeSkIdRoundtrip)
{
  Handle(ExtrudeFeature) e = new ExtrudeFeature();
  e->setDistance(7.0);
  e->setSketchId(1234);
  const std::string blob = e->serialize();

  Handle(ExtrudeFeature) f = new ExtrudeFeature();
  f->deserialize(blob);
  EXPECT_EQ(f->sketchId(), 1234u);
}

