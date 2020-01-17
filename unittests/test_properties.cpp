#include "task_manager/property.h"
#include "task_manager/conversion.h"
#include "task_manager/constraint.h"
#include "task_manager/constraint_grammar.h"

#include "gtest/gtest.h"

namespace
{
  struct Scope
  {};

  struct SCustomType
  {
    int iVal = 0;
    double dVal = 0;
    QString sVal;
    bool bVal = false;

    bool operator==(const SCustomType& other) const
    {
      return iVal == other.iVal &&
          dVal == other.dVal &&
          sVal == other.sVal &&
          bVal == other.bVal;
    }
  };
}

namespace conversion
{
  template<>
  QString toString<SCustomType>(const SCustomType& t)
  {
    return QString();
  }

  template<>
  SCustomType fromString<SCustomType>(const QString& s, bool& /*bStatus*/)
  {
    return SCustomType();
  }
}

TEST(PropertiesTest, Set_FailIfUnknownProperty)
{
  Properties<Scope> props;
  EXPECT_FALSE(props.set("name", 123));
}

TEST(PropertiesTest, Set)
{
  Properties<Scope> props;

  REGISTER_PROPERTY(Scope, "name", QString, false);

  EXPECT_TRUE(props.set("name", QString("hello")));
}

TEST(PropertiesTest, Get)
{
  Properties<Scope> props;

  REGISTER_PROPERTY(Scope, "name", int, false);

  props.set("name", 123);
  EXPECT_EQ(123, props.get<int>("name"));
}

TEST(PropertiesTest, Get_ConvertIfWrongType)
{
  Properties<Scope> props;

  REGISTER_PROPERTY(Scope, "name", int, false);

  props.set("name", 123);
  EXPECT_EQ("123", props.get<QString>("name"));
  EXPECT_DOUBLE_EQ(123, props.get<double>("name"));
}

TEST(PropertiesTest, Get_FailIfNotConvertible)
{
  Properties<Scope> props;

  REGISTER_PROPERTY(Scope, "name", QString, false);

  props.set("name", QString("hello world"));

  // "hello world" cannot be converted to int,
  // so return a default-constructed int (0)
  EXPECT_EQ(0, props.get<int>("name"));
}

TEST(PropertiesTest, Set_WithCustomType)
{
  Properties<Scope> props;

  REGISTER_PROPERTY(Scope, "custom", SCustomType, false);

  SCustomType typeInstance;
  EXPECT_TRUE(props.set("custom", typeInstance));
}

TEST(PropertiesTest, Set_WithCustomType_FailIfNotConvertible)
{
  Properties<Scope> props;

  REGISTER_PROPERTY(Scope, "custom", SCustomType, false);

  EXPECT_FALSE(props.set("custom", QString("hello world")));
}


TEST(PropertiesTest, Set_WithMinMaxConstraint)
{
  using namespace grammar;

  struct ScopeWithMinMaxConstraints {};

  Properties<ScopeWithMinMaxConstraints> props;

  REGISTER_PROPERTY(ScopeWithMinMaxConstraints, "value", int, false);

  Properties<ScopeWithMinMaxConstraints>::registerConstraint("value", MIN(0) AND MAX(100));

  EXPECT_TRUE(props.set("value", 0));
  EXPECT_TRUE(props.set("value", 100));
  EXPECT_FALSE(props.set("value", -1));
  EXPECT_FALSE(props.set("value", 101));
}

TEST(PropertiesTest, Set_WithMinMaxConstraintDouble)
{
  using namespace grammar;

  struct ScopeWithMinMaxConstraintsDouble {};

  Properties<ScopeWithMinMaxConstraintsDouble> props;

  REGISTER_PROPERTY(ScopeWithMinMaxConstraintsDouble, "value", double, false);

  Properties<ScopeWithMinMaxConstraintsDouble>::registerConstraint("value", MIN(0.5) AND MAX(99.5));

  EXPECT_TRUE(props.set("value", 0.8));
  EXPECT_TRUE(props.set("value", 99.2));
  EXPECT_FALSE(props.set("value", 0.4));
  EXPECT_FALSE(props.set("value", 99.55));
}

TEST(PropertiesTest, Set_WithOneOfStringConstraint)
{
  using namespace grammar;

  struct ScopeWithOneOfStringConstraints {};

  Properties<ScopeWithOneOfStringConstraints> props;

  REGISTER_PROPERTY(ScopeWithOneOfStringConstraints, "name", QString, false);

  Properties<ScopeWithOneOfStringConstraints>::registerConstraint("name", ONE_OF(QString("a"), QString("b"), QString("c")));

  EXPECT_TRUE(props.set("name", QString("a")));
  EXPECT_TRUE(props.set("name", QString("b")));
  EXPECT_TRUE(props.set("name", QString("c")));
  EXPECT_FALSE(props.set("name", QString("d")));
  EXPECT_FALSE(props.set("name", QString("hello world")));
}

TEST(PropertiesTest, Set_WithOneOfIntConstraint)
{
  using namespace grammar;

  struct ScopeWithOneOfIntConstraints {};

  Properties<ScopeWithOneOfIntConstraints> props;

  REGISTER_PROPERTY(ScopeWithOneOfIntConstraints, "value", int, false);

  Properties<ScopeWithOneOfIntConstraints>::registerConstraint("value", ONE_OF(1, 3, 10, 240, -455));

  EXPECT_TRUE(props.set("value", 1));
  EXPECT_TRUE(props.set("value", 3));
  EXPECT_TRUE(props.set("value", 10));
  EXPECT_TRUE(props.set("value", 240));
  EXPECT_TRUE(props.set("value", -455));
  EXPECT_FALSE(props.set("value",2));
  EXPECT_FALSE(props.set("value",-500));
}

TEST(PropertiesTest, Set_WithOneOfIntConstraint2)
{
  using namespace grammar;

  struct ScopeWithOneOfIntConstraints {};

  Properties<ScopeWithOneOfIntConstraints> props;

  REGISTER_PROPERTY(ScopeWithOneOfIntConstraints, "value", int, false);

  Properties<ScopeWithOneOfIntConstraints>::registerConstraint("value", ONE_OF(1, 3, 10) OR ONE_OF(2, 4, 6));

  EXPECT_TRUE(props.set("value", 1));
  EXPECT_TRUE(props.set("value", 3));
  EXPECT_TRUE(props.set("value", 10));
  EXPECT_TRUE(props.set("value",2));
  EXPECT_TRUE(props.set("value",4));
  EXPECT_TRUE(props.set("value",6));
  EXPECT_FALSE(props.set("value",-500));
}















