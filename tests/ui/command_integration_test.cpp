#include <gtest/gtest.h>

#include <Document.h>
#include <command/CreateBoxCommand.h>
#include <command/CreateCylinderCommand.h>
#include <command/CreateExtrudeCommand.h>
#include <ExtrudeFeature.h>
#include <Sketch.h>

TEST(UICommands, CreateBoxAddsFeature) {
  Document doc;
  EXPECT_EQ(doc.features().Size(), 0);

  CreateBoxCommand cmd(10.0, 20.0, 30.0);
  cmd.execute(doc);

  EXPECT_EQ(doc.features().Size(), 1);
  auto f = doc.features().First();
  EXPECT_FALSE(f.IsNull());
}

TEST(UICommands, RemoveLastFeature) {
  Document doc;
  CreateBoxCommand cmd1(10.0, 20.0, 30.0);
  CreateBoxCommand cmd2(5.0, 6.0, 7.0);
  cmd1.execute(doc);
  cmd2.execute(doc);
  ASSERT_EQ(doc.features().Size(), 2);
  doc.removeLast();
  EXPECT_EQ(doc.features().Size(), 1);
}

TEST(UICommands, CreateCylinderAddsFeature) {
  Document doc;
  EXPECT_EQ(doc.features().Size(), 0);

  CreateCylinderCommand cmd(8.0, 15.0);
  cmd.execute(doc);

  EXPECT_EQ(doc.features().Size(), 1);
  auto f = doc.features().First();
  EXPECT_FALSE(f.IsNull());
}

TEST(UICommands, CreateExtrudeResolvesSketchById) {
  Document doc;
  // Build a simple rectangle sketch
  auto sk = std::make_shared<Sketch>();
  const double w = 12.0, h = 7.0, d = 5.0;
  auto c1 = sk->addLine(gp_Pnt2d(0.0, 0.0), gp_Pnt2d(w, 0.0));
  auto c2 = sk->addLine(gp_Pnt2d(w, 0.0), gp_Pnt2d(w, h));
  auto c3 = sk->addLine(gp_Pnt2d(w, h), gp_Pnt2d(0.0, h));
  auto c4 = sk->addLine(gp_Pnt2d(0.0, h), gp_Pnt2d(0.0, 0.0));
  sk->addCoincident({c1,1}, {c2,0});
  sk->addCoincident({c2,1}, {c3,0});
  sk->addCoincident({c3,1}, {c4,0});
  sk->addCoincident({c4,1}, {c1,0});
  sk->solveConstraints();

  CreateExtrudeCommand cmd(sk, d);
  cmd.execute(doc);

  ASSERT_EQ(doc.features().Size(), 1);
  Handle(Feature) base = doc.features().First();
  Handle(ExtrudeFeature) ef = Handle(ExtrudeFeature)::DownCast(base);
  ASSERT_FALSE(ef.IsNull());
  EXPECT_EQ(ef->sketchId(), sk->id());
  // Document::recompute was called inside the command; extrude should have a resolved sketch and a valid solid
  ASSERT_TRUE(static_cast<bool>(ef->sketch()));
  const auto& shp = ef->shape();
  ASSERT_FALSE(shp.IsNull());
  EXPECT_EQ(shp.ShapeType(), TopAbs_SOLID);
}
