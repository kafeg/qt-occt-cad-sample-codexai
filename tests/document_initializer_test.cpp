#include <gtest/gtest.h>

#include <Document.h>
#include <PlaneFeature.h>
#include <PointFeature.h>
#include <AxeFeature.h>
#include <Datum.h>

TEST(DocumentInitializer, NewDocumentHasDefaultDatumGeometry)
{
  Document doc;

  // Count planes, axes and points in the ordered items list
  int planeCount = 0;
  int axisCount  = 0;
  int pointCount = 0;
  for (NCollection_Sequence<Handle(DocumentItem)>::Iterator it(doc.items()); it.More(); it.Next())
  {
    const Handle(DocumentItem)& di = it.Value();
    if (!Handle(PlaneFeature)::DownCast(di).IsNull())
      ++planeCount;
    if (!Handle(AxeFeature)::DownCast(di).IsNull())
      ++axisCount;
    if (!Handle(PointFeature)::DownCast(di).IsNull())
      ++pointCount;
  }
  EXPECT_EQ(planeCount, 3) << "Document should initialize XY/XZ/YZ planes";
  EXPECT_EQ(axisCount, 3) << "Document should initialize X/Y/Z axes";

  // By default Datum shows the origin point
  ASSERT_TRUE(doc.datum()->showOriginPoint());
  EXPECT_EQ(pointCount, 1) << "Document should initialize origin PointFeature when enabled";

  // features() should exclude helper planes, axes and origin point
  EXPECT_EQ(doc.features().Size(), 0);
}

TEST(DocumentInitializer, ClearReinitializesUsingDatumToggles)
{
  Document doc;
  auto     d = doc.datum();
  ASSERT_TRUE(static_cast<bool>(d));

  // Hide origin and X axis, then clear: no PointFeature and X axis suppressed
  d->setShowOriginPoint(false);
  d->setShowTrihedronAxisX(false);
  doc.clear();
  int pointCount     = 0;
  int axisCount      = 0;
  int suppressedAxes = 0;
  for (NCollection_Sequence<Handle(DocumentItem)>::Iterator it(doc.items()); it.More(); it.Next())
  {
    const Handle(DocumentItem)& di = it.Value();
    if (!Handle(PointFeature)::DownCast(di).IsNull())
      ++pointCount;
    if (Handle(AxeFeature) ax = Handle(AxeFeature)::DownCast(di); !ax.IsNull())
    {
      ++axisCount;
      if (ax->isSuppressed())
        ++suppressedAxes;
    }
  }
  EXPECT_EQ(pointCount, 0) << "Origin point should not be present when disabled";
  EXPECT_EQ(axisCount, 3);
  EXPECT_EQ(suppressedAxes, 1) << "X axis should be suppressed when disabled";

  // Show origin and X axis and clear: PointFeature present and axes unsuppressed
  d->setShowOriginPoint(true);
  d->setShowTrihedronAxisX(true);
  doc.clear();
  pointCount     = 0;
  int planeCount = 0;
  axisCount      = 0;
  suppressedAxes = 0;
  for (NCollection_Sequence<Handle(DocumentItem)>::Iterator it2(doc.items()); it2.More(); it2.Next())
  {
    const Handle(DocumentItem)& di = it2.Value();
    if (!Handle(PointFeature)::DownCast(di).IsNull())
      ++pointCount;
    if (!Handle(PlaneFeature)::DownCast(di).IsNull())
      ++planeCount;
    if (Handle(AxeFeature) ax = Handle(AxeFeature)::DownCast(di); !ax.IsNull())
    {
      ++axisCount;
      if (ax->isSuppressed())
        ++suppressedAxes;
    }
  }
  EXPECT_EQ(pointCount, 1);
  EXPECT_EQ(planeCount, 3);
  EXPECT_EQ(axisCount, 3);
  EXPECT_EQ(suppressedAxes, 0);
}
