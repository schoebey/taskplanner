#include "gtest/gtest.h"
#include "gtest_helpers.h"
#include "libtaskmanager/conversion.h"

#include <QDateTime>

std::ostream& operator<<(std::ostream& o, const QString& s)
{
  o << s.toUtf8().constData();
  return o;
}

TEST(Conversion, int_fromString)
{
  bool bConversionStatus(false);
  int i0 = conversion::fromString<int>("42", bConversionStatus);
  EXPECT_EQ(42, i0);
  EXPECT_TRUE(bConversionStatus);

  int i1 = conversion::fromString<int>("742", bConversionStatus);
  EXPECT_EQ(742, i1);
  EXPECT_TRUE(bConversionStatus);

  int i2 = conversion::fromString<int>("4711", bConversionStatus);
  EXPECT_EQ(4711, i2);
  EXPECT_TRUE(bConversionStatus);

  int i3 = conversion::fromString<int>("4711002", bConversionStatus);
  EXPECT_EQ(4711002, i3);
  EXPECT_TRUE(bConversionStatus);
}

TEST(Conversion, int_fromString_negative)
{
  bool bConversionStatus(false);
  int i0 = conversion::fromString<int>("-42", bConversionStatus);
  EXPECT_EQ(-42, i0);
  EXPECT_TRUE(bConversionStatus);

  int i1 = conversion::fromString<int>("-742", bConversionStatus);
  EXPECT_EQ(-742, i1);
  EXPECT_TRUE(bConversionStatus);

  int i2 = conversion::fromString<int>("-4711", bConversionStatus);
  EXPECT_EQ(-4711, i2);
  EXPECT_TRUE(bConversionStatus);

  int i3 = conversion::fromString<int>("-4711002", bConversionStatus);
  EXPECT_EQ(-4711002, i3);
  EXPECT_TRUE(bConversionStatus);
}

TEST(Conversion, int_toString)
{
  QString s0 = conversion::toString(42);
  EXPECT_EQ("42", s0);

  QString s1 = conversion::toString(742);
  EXPECT_EQ("742", s1);

  QString s2 = conversion::toString(4711);
  EXPECT_EQ("4711", s2);

  QString s3 = conversion::toString(4711002);
  EXPECT_EQ("4711002", s3);
}

TEST(Conversion, int_toString_negative)
{
  QString s0 = conversion::toString(-42);
  EXPECT_EQ("-42", s0);

  QString s1 = conversion::toString(-742);
  EXPECT_EQ("-742", s1);

  QString s2 = conversion::toString(-4711);
  EXPECT_EQ("-4711", s2);

  QString s3 = conversion::toString(-4711002);
  EXPECT_EQ("-4711002", s3);
}

TEST(Conversion, fancy_int_fromString)
{
  bool bIsInt = false;
  int iTest = conversion::fancy::toInt("ninehundredfiftytwo", &bIsInt);
  EXPECT_TRUE(bIsInt);
  EXPECT_EQ(iTest, 952);

  iTest = conversion::fancy::toInt("fiftyonethousandsevenhundredandfive", &bIsInt);
  EXPECT_TRUE(bIsInt);
  EXPECT_EQ(iTest, 51705);

  iTest = conversion::fancy::toInt("seventytwomillionandthree", &bIsInt);
  EXPECT_TRUE(bIsInt);
  EXPECT_EQ(iTest, 72000003);

  iTest = conversion::fancy::toInt("seventy two million and three", &bIsInt);
  EXPECT_TRUE(bIsInt);
  EXPECT_EQ(iTest, 72000003);
}

TEST(Conversion, fancy_int_fromString_failIfNotANumber)
{
  bool bIsInt = false;
  conversion::fancy::toInt("hello world", &bIsInt);
  EXPECT_FALSE(bIsInt);

  conversion::fancy::toInt("fourtytwo", &bIsInt);
  EXPECT_FALSE(bIsInt);

  conversion::fancy::toInt("fortytoo", &bIsInt);
  EXPECT_FALSE(bIsInt);
}

TEST(Conversion, fancy_int_fromString_negative)
{
  bool bIsInt = false;
  int iTest = conversion::fancy::toInt("minusninehundredfiftytwo", &bIsInt);
  EXPECT_TRUE(bIsInt);
  EXPECT_EQ(iTest, -952);

  iTest = conversion::fancy::toInt("negativefiftyonethousandsevenhundredandfive", &bIsInt);
  EXPECT_TRUE(bIsInt);
  EXPECT_EQ(iTest, -51705);

  iTest = conversion::fancy::toInt("minus seventy two million and three", &bIsInt);
  EXPECT_TRUE(bIsInt);
  EXPECT_EQ(iTest, -72000003);
}

TEST(Conversion, fancy_int_toString)
{
  QString sFortyTwo = conversion::fancy::toString(42);
  EXPECT_EQ("fortytwo", sFortyTwo);

  QString s7FortyTwo = conversion::fancy::toString(742);
  EXPECT_EQ("sevenhundredandfortytwo", s7FortyTwo);

  QString sTest = conversion::fancy::toString(4711);
  EXPECT_EQ("fourthousandsevenhundredandeleven", sTest);

  QString s2005 = conversion::fancy::toString(2005);
  EXPECT_EQ("twothousandandfive", s2005);

  QString sTest2 = conversion::fancy::toString(4711002);
  EXPECT_EQ("fourmillionsevenhundredandeleventhousandandtwo", sTest2);
}

TEST(Conversion, double_fromString)
{
  bool bConversionStatus(false);
  double d0 = conversion::fromString<double>("3.141", bConversionStatus);
  EXPECT_DOUBLE_EQ(3.141, d0);
  EXPECT_TRUE(bConversionStatus);

  double d1 = conversion::fromString<double>("0.07", bConversionStatus);
  EXPECT_DOUBLE_EQ(0.07, d1);
  EXPECT_TRUE(bConversionStatus);
}

TEST(Conversion, double_fromString_negative)
{
  bool bConversionStatus(false);
  double d0 = conversion::fromString<double>("-3.141", bConversionStatus);
  EXPECT_DOUBLE_EQ(-3.141, d0);
  EXPECT_TRUE(bConversionStatus);

  double d1 = conversion::fromString<double>("-0.07", bConversionStatus);
  EXPECT_DOUBLE_EQ(-0.07, d1);
  EXPECT_TRUE(bConversionStatus);
}

TEST(Conversion, double_toString)
{
  QString s0 = conversion::toString(3.141);
  EXPECT_EQ("3.141", s0);

  QString s1 = conversion::toString(0.07);
  EXPECT_EQ("0.07", s1);
}

TEST(Conversion, double_toString_negative)
{
  QString s0 = conversion::toString(-3.141);
  EXPECT_EQ("-3.141", s0);

  QString s1 = conversion::toString(-0.07);
  EXPECT_EQ("-0.07", s1);
}


namespace
{
class DateTimeConversionTest : public ::testing::Test {
protected:
  DateTimeConversionTest() {
  }

  ~DateTimeConversionTest() override {
  }

  void SetUp() override {
    m_baseDateTime = QDateTime::fromString("2000-05-01T00:00:00.000", Qt::ISODate);
    m_baseDateTimeEndOfYear = QDateTime::fromString("1999-12-31T23:59:59.999", Qt::ISODate);
  }

  void TearDown() override {
  }

  QDateTime m_baseDateTime;
  QDateTime m_baseDateTimeEndOfYear;
};
}

TEST_F(DateTimeConversionTest, date_fromString_nextWeekDay)
{
  std::vector<QString> vsDayNames = {"monday", "tuesday", "wednesday", "thursday", "friday", "saturday", "sunday"};
  std::vector<QString> vsDayNamesShort = {"mon", "tue", "wed", "thu", "fri", "sat", "sun"};

  for (int i = 0; i < vsDayNames.size(); ++i)
  {
    bool bStatus(false);
    QDateTime dt = conversion::dateTimeFromString(QString("next %1").arg(vsDayNames[i]), bStatus, m_baseDateTime);
    EXPECT_TRUE(bStatus);
    EXPECT_EQ(dt.date().dayOfWeek(), i + 1);
    EXPECT_GT(dt.date().dayOfYear(), m_baseDateTime.date().dayOfYear());


    dt = conversion::dateTimeFromString(QString("next %1").arg(vsDayNamesShort[i]), bStatus, m_baseDateTime);
    EXPECT_TRUE(bStatus);
    EXPECT_EQ(dt.date().dayOfWeek(), i + 1);
    EXPECT_GT(dt.date().dayOfYear(), m_baseDateTime.date().dayOfYear());
  }
}

TEST_F(DateTimeConversionTest, date_fromString_nextWeek)
{
  bool bStatus(false);
  QDateTime dt = conversion::dateTimeFromString("next week", bStatus, m_baseDateTime);
  EXPECT_TRUE(bStatus);
  EXPECT_EQ(dt.date().dayOfWeek(), m_baseDateTime.date().dayOfWeek());
  EXPECT_EQ(dt.date().weekNumber(), m_baseDateTime.date().weekNumber() + 1);

  // if at the end of the year, we expect a wraparound
  dt = conversion::dateTimeFromString("next week", bStatus, m_baseDateTimeEndOfYear);
  EXPECT_TRUE(bStatus);
  EXPECT_EQ(dt.date().dayOfWeek(), m_baseDateTimeEndOfYear.date().dayOfWeek());
  EXPECT_EQ(1, dt.date().weekNumber());
}

TEST_F(DateTimeConversionTest, date_fromString_nextMonth)
{
  bool bStatus(false);
  QDateTime dt = conversion::dateTimeFromString("next month", bStatus, m_baseDateTime);
  EXPECT_TRUE(bStatus);
  EXPECT_EQ(dt.date().day(), m_baseDateTime.date().day());
  EXPECT_EQ(dt.date().month(), m_baseDateTime.date().month() + 1);

  // if at the end of the year, we expect a wraparound
  dt = conversion::dateTimeFromString("next month", bStatus, m_baseDateTimeEndOfYear);
  EXPECT_TRUE(bStatus);
  EXPECT_EQ(dt.date().day(), m_baseDateTimeEndOfYear.date().day());
  EXPECT_EQ(1, dt.date().month());
}

TEST_F(DateTimeConversionTest, date_fromString_nextMonthName)
{
  std::vector<QString> vsMonthNames = {"january", "february", "march", "april", "may", "june", "july",
                                       "august", "september", "october", "november", "december"};
  std::vector<QString> vsMonthNamesShort = {"jan", "feb", "mar", "apr", "may", "jun", "jul",
                                            "aug", "sep", "oct", "nov", "dec"};

  for (int i = 0; i < vsMonthNames.size(); ++i)
  {
    bool bStatus(false);
    QDateTime dt = conversion::dateTimeFromString(QString("next %1").arg(vsMonthNames[i]), bStatus, m_baseDateTime);
    EXPECT_TRUE(bStatus) << vsMonthNames[i];
    EXPECT_EQ(dt.date().month(), i + 1) << vsMonthNames[i];
    EXPECT_GT(dt, m_baseDateTime) << vsMonthNames[i];


    dt = conversion::dateTimeFromString(QString("next %1").arg(vsMonthNamesShort[i]), bStatus, m_baseDateTime);
    EXPECT_TRUE(bStatus) << vsMonthNamesShort[i];
    EXPECT_EQ(dt.date().month(), i + 1) << vsMonthNamesShort[i];
    EXPECT_GT(dt, m_baseDateTime) << vsMonthNamesShort[i];
  }
}


TEST_F(DateTimeConversionTest, date_fromString_relativeOffset_weeks)
{
  bool bStatus(false);
  QDateTime dt = conversion::dateTimeFromString("in one week", bStatus, m_baseDateTime);
  EXPECT_TRUE(bStatus);
  EXPECT_EQ(dt.date().weekNumber(), m_baseDateTime.date().weekNumber() + 1);

  dt = conversion::dateTimeFromString("in 1 week", bStatus, m_baseDateTime);
  EXPECT_TRUE(bStatus);
  EXPECT_EQ(dt.date().weekNumber(), m_baseDateTime.date().weekNumber() + 1);

  dt = conversion::dateTimeFromString("in five weeks", bStatus, m_baseDateTime);
  EXPECT_TRUE(bStatus);
  EXPECT_EQ(dt.date().weekNumber(), m_baseDateTime.date().weekNumber() + 5);

  dt = conversion::dateTimeFromString("in 5 weeks", bStatus, m_baseDateTime);
  EXPECT_TRUE(bStatus);
  EXPECT_EQ(dt.date().weekNumber(), m_baseDateTime.date().weekNumber() + 5);
}

TEST_F(DateTimeConversionTest, date_fromString_relativeOffset_days)
{
  bool bStatus(false);
  QDateTime dt = conversion::dateTimeFromString("in one day", bStatus, m_baseDateTime);
  EXPECT_TRUE(bStatus);
  EXPECT_EQ(dt.date().dayOfYear(), m_baseDateTime.date().dayOfYear() + 1);

  dt = conversion::dateTimeFromString("in 1 day", bStatus, m_baseDateTime);
  EXPECT_TRUE(bStatus);
  EXPECT_EQ(dt.date().dayOfYear(), m_baseDateTime.date().dayOfYear() + 1);

  dt = conversion::dateTimeFromString("in five days", bStatus, m_baseDateTime);
  EXPECT_TRUE(bStatus);
  EXPECT_EQ(dt.date().dayOfYear(), m_baseDateTime.date().dayOfYear() + 5);

  dt = conversion::dateTimeFromString("in 5 days", bStatus, m_baseDateTime);
  EXPECT_TRUE(bStatus);
  EXPECT_EQ(dt.date().dayOfYear(), m_baseDateTime.date().dayOfYear() + 5);
}


