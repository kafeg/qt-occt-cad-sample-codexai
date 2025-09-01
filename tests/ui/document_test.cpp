#include <gtest/gtest.h>

#include <Document.h>
#include <command/CreateBoxCommand.h>
#include <BoxFeature.h>

// These tests model the idea of tabs: independent documents.

TEST(TabsModel, IndependentDocuments)
{
  Document docA;
  Document docB;

  EXPECT_EQ(docA.features().Size(), 0);
  EXPECT_EQ(docB.features().Size(), 0);

  Handle(BoxFeature) a = new BoxFeature();
  a->setSize(1.0, 2.0, 3.0);
  docA.addFeature(a);
  docA.recompute();

  EXPECT_EQ(docA.features().Size(), 1);
  EXPECT_EQ(docB.features().Size(), 0);

  Handle(BoxFeature) b = new BoxFeature();
  b->setSize(2.0, 3.0, 4.0);
  docB.addFeature(b);
  docB.recompute();

  EXPECT_EQ(docA.features().Size(), 1);
  EXPECT_EQ(docB.features().Size(), 1);
}

TEST(TabsModel, CommandsApplyPerDocument)
{
  Document doc1;
  Document doc2;

  CreateBoxCommand c1(10.0, 20.0, 30.0);
  CreateBoxCommand c2(5.0, 5.0, 5.0);

  c1.execute(doc1);
  EXPECT_EQ(doc1.features().Size(), 1);
  EXPECT_EQ(doc2.features().Size(), 0);

  c2.execute(doc2);
  EXPECT_EQ(doc1.features().Size(), 1);
  EXPECT_EQ(doc2.features().Size(), 1);
}
