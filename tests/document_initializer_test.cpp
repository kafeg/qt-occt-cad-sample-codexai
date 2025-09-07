#include <gtest/gtest.h>

#include <Document.h>
#include <PlaneFeature.h>
#include <PointFeature.h>
#include <Datum.h>

TEST(DocumentInitializer, NewDocumentHasDefaultPlanesAndOptionalOrigin)
{
  Document doc;

  // Count planes and points in the ordered items list
  int planeCount = 0;
  int pointCount = 0;
  for (NCollection_Sequence<Handle(DocumentItem)>::Iterator it(doc.items()); it.More(); it.Next())
  {
    const Handle(DocumentItem)& di = it.Value();
    if (!Handle(PlaneFeature)::DownCast(di).IsNull()) ++planeCount;
    if (!Handle(PointFeature)::DownCast(di).IsNull()) ++pointCount;
  }
  EXPECT_EQ(planeCount, 3) << "Document should initialize XY/XZ/YZ planes";

  // By default Datum shows the origin point
  ASSERT_TRUE(doc.datum()->showOriginPoint());
  EXPECT_EQ(pointCount, 1) << "Document should initialize origin PointFeature when enabled";

  // features() should exclude helper planes and origin point
  EXPECT_EQ(doc.features().Size(), 0);
}

TEST(DocumentInitializer, ClearReinitializesUsingDatumToggles)
{
  Document doc;
  auto d = doc.datum();
  ASSERT_TRUE(static_cast<bool>(d));

  // Hide origin and clear: no PointFeature expected
  d->setShowOriginPoint(false);
  doc.clear();
  int pointCount = 0;
  for (NCollection_Sequence<Handle(DocumentItem)>::Iterator it(doc.items()); it.More(); it.Next())
  {
    if (!Handle(PointFeature)::DownCast(it.Value()).IsNull()) ++pointCount;
  }
  EXPECT_EQ(pointCount, 0) << "Origin point should not be present when disabled";

  // Show origin and clear: PointFeature expected
  d->setShowOriginPoint(true);
  doc.clear();
  pointCount = 0;
  int planeCount = 0;
  for (NCollection_Sequence<Handle(DocumentItem)>::Iterator it2(doc.items()); it2.More(); it2.Next())
  {
    if (!Handle(PointFeature)::DownCast(it2.Value()).IsNull()) ++pointCount;
    if (!Handle(PlaneFeature)::DownCast(it2.Value()).IsNull()) ++planeCount;
  }
  EXPECT_EQ(pointCount, 1);
  EXPECT_EQ(planeCount, 3);
}

