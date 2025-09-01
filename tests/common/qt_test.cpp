#include <gtest/gtest.h>

#include <QString>
#include <QVersionNumber>

TEST(QtCore, QStringToUpper) {
  QString s = "occt";
  EXPECT_EQ(s.toUpper().toStdString(), std::string("OCCT"));
}

TEST(QtCore, VersionParse) {
  auto v = QVersionNumber::fromString(QString::fromUtf8(QT_VERSION_STR));
  EXPECT_TRUE(v.majorVersion() >= 6);
}

