#include <gtest/gtest.h>

#include <Document.h>
#include <CreateBoxCommand.h>
#include <CreateCylinderCommand.h>

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

