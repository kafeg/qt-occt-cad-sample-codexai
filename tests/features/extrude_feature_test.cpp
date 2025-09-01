#include <gtest/gtest.h>

#include <ExtrudeFeature.h>
#include <Sketch.h>

#include <TopAbs_ShapeEnum.hxx>
#include <common/test_utils.h>

TEST(Model, ExtrudeFeatureSolidBBoxAndVolume)
{
  // Build a simple rectangular profile 10 x 20 in XY
  auto sk = std::make_shared<Sketch>();
  const double w = 10.0;
  const double h = 20.0;

  // Rectangle corners: (0,0) -> (w,0) -> (w,h) -> (0,h) -> (0,0)
  Sketch::CurveId c1 = sk->addLine(gp_Pnt2d(0.0, 0.0), gp_Pnt2d(w, 0.0));
  Sketch::CurveId c2 = sk->addLine(gp_Pnt2d(w, 0.0), gp_Pnt2d(w, h));
  Sketch::CurveId c3 = sk->addLine(gp_Pnt2d(w, h), gp_Pnt2d(0.0, h));
  Sketch::CurveId c4 = sk->addLine(gp_Pnt2d(0.0, h), gp_Pnt2d(0.0, 0.0));

  // Enforce coincident endpoints around the loop
  sk->addCoincident({c1, 1}, {c2, 0});
  sk->addCoincident({c2, 1}, {c3, 0});
  sk->addCoincident({c3, 1}, {c4, 0});
  sk->addCoincident({c4, 1}, {c1, 0});

  sk->solveConstraints();

  // Extrude by 30 along +Z
  const double d = 30.0;
  Handle(ExtrudeFeature) ef = new ExtrudeFeature();
  ef->setSketch(sk);
  ef->setDistance(d);
  ef->execute();

  const auto& shp = ef->shape();
  ASSERT_FALSE(shp.IsNull());
  EXPECT_EQ(shp.ShapeType(), TopAbs_SOLID);

  // Bounding box must match w x h x d
  const double tol = 1e-6;
  auto ext = bboxExtents(shp);
  EXPECT_NEAR(ext[0], w, tol);
  EXPECT_NEAR(ext[1], h, tol);
  EXPECT_NEAR(ext[2], d, tol);

  // Volume check
  EXPECT_NEAR(volume(shp), w * h * d, 1e-6);
}

