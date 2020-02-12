#include "gtest/gtest.h"
#include "gtest_helpers.h"
#include "libtaskmanager/conversion.h"



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


