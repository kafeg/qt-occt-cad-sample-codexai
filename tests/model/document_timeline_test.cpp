#include <gtest/gtest.h>

#include <Document.h>
#include <BoxFeature.h>
#include <CylinderFeature.h>

TEST(DocumentTimeline, AddInsertAndIterateItems)
{
  Document doc;
  EXPECT_EQ(doc.items().Size(), 0);
  EXPECT_EQ(doc.features().Size(), 0);

  Handle(BoxFeature) a = new BoxFeature();
  a->setSize(1.0, 2.0, 3.0);

  Handle(CylinderFeature) b = new CylinderFeature();
  b->set(4.0, 5.0);

  // Append A, then insert B at the front
  doc.addItem(Handle(DocumentItem)::DownCast(a));
  doc.insertItem(1, Handle(DocumentItem)::DownCast(b));

  ASSERT_EQ(doc.items().Size(), 2);
  // Items are in order: B, A
  {
    auto first = doc.items().Value(1);
    auto second = doc.items().Value(2);
    EXPECT_TRUE(!Handle(Feature)::DownCast(first).IsNull());
    EXPECT_TRUE(!Handle(Feature)::DownCast(second).IsNull());
    // features() mirrors that order filtered by kind
    ASSERT_EQ(doc.features().Size(), 2);
    EXPECT_TRUE(Handle(CylinderFeature)::DownCast(doc.features().Value(1)) == b);
    EXPECT_TRUE(Handle(BoxFeature)::DownCast(doc.features().Value(2)) == a);
  }

  // Remove last (A)
  doc.removeLast();
  ASSERT_EQ(doc.items().Size(), 1);
  ASSERT_EQ(doc.features().Size(), 1);
  EXPECT_TRUE(Handle(CylinderFeature)::DownCast(doc.features().Value(1)) == b);

  // Remove specific feature (B)
  doc.removeFeature(b);
  EXPECT_EQ(doc.items().Size(), 0);
  EXPECT_EQ(doc.features().Size(), 0);
}

