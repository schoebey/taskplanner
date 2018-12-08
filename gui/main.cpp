#include <QApplication>

#include "mainwindow.h"
#include "manager.h"
#include "task.h"
#include "group.h"
#include "serializerfactory.h"
#include "style.h"
#include "property.h"
#include "constraint.h"

#include <QDateTime>

#include <cassert>

namespace grammar
{
  // logical
  #define AND &
  #define BUT AND
  #define OR |

  // constraints
  #define ANY_OF(a, ...) tspConstraint<decltype(a)>(make_list<decltype(a)>(a, ## __VA_ARGS__))
  #define MIN(a) tspConstraint<decltype(a)>(std::make_shared<MinConstraint<decltype(a)>>(a))
  #define MAX(a) tspConstraint<decltype(a)>(std::make_shared<MaxConstraint<decltype(a)>>(a))
  #define EVEN_NUMBER std::make_shared<ConstraintImpl>()


  template<typename T> tspConstraint<T> operator&(const std::shared_ptr<ConstraintTpl<T>>& p,
                                                  const std::shared_ptr<ConstraintTpl<T>>& p2)
  {
    return std::make_shared<AndConstraint<T>>(p, p2);
  }
  template<typename T> tspConstraint<T> operator|(tspConstraint<T> p, tspConstraint<T> p2)
  {
    return std::make_shared<OrConstraint<T>>(p, p2);
  }
}


int main(int argc, char *argv[])
{
  // test code for constraints
  using namespace grammar;

  //auto a = ListConstraintTpl<int>(1,2,3,4,5,6);
  //auto a = make_list(1,2,3,4,5);
//  auto b = ANY_OF(1,2,3);
  auto sp = ANY_OF(1,2,3) OR MIN(10) AND MAX(20);
  auto sp2 = ANY_OF(1.5,2.,3.) OR MIN(10.) AND MAX(20.);
  auto sp25 = ANY_OF(QString("i"), QString("am"), QString("ready"));
  auto sp3 = ANY_OF(QString("hello"), QString("world"), QString("abc")) OR ANY_OF(QString("i"), QString("am"), QString("ready"));

  bool bOk = sp->accepts(12);
  bOk = sp->accepts(8);
  bOk = sp->accepts(22);
  bOk = sp->accepts(1);
  bOk = sp->accepts(2);
  bOk = sp->accepts(3);
  bOk = sp3->accepts("hello");
  bOk = sp3->accepts("wrold");


//  tspConstraint spResult = ANY_OF('a', 'b', 'c') OR (EVEN_NUMBER BUT MIN(0) AND MAX(100));


  // test code for properties
  REGISTER_PROPERTY("name", QString, false);
  REGISTER_PROPERTY("description", QString, false);
  REGISTER_PROPERTY("expanded", bool, false);
  REGISTER_PROPERTY("due date", QDateTime, true);
  REGISTER_PROPERTY("category", QString, true);

//  TODO: customizable enum property (e.g. 'category' with n possible values)
//  typedef std::array<QString, 3> ar;
//  REGISTER_PROPERTY("test", ar, true);

//  TODO: configure order of comparison for properties, e.g.:
//    (due date - expected duration), priority(high,med,low), category(feature,bugfix,refactoring,documentation)

  Properties props;
  props.set("due date", QDateTime::currentDateTime());
  props.get<QDateTime>("due date");


  QApplication app(argc, argv);

  app.setStyle(new Style());

  Manager manager;


  tspSerializer spTextWriter = SerializerFactory::create("markdown");
  if (!spTextWriter->setParameter("fileName", "serialisation_markdown.txt"))
  {
    assert(false);
  }

  EDeserializingError de = manager.deserializeFrom(spTextWriter.get());


  if (EDeserializingError::eOk != de || 0 == manager.groupIds().size())
  {
      IGroup* pGroup = manager.addGroup();
      pGroup->setName("backlog");

      pGroup = manager.addGroup();
      pGroup->setName("in progress");

      pGroup = manager.addGroup();
      pGroup->setName("done");
  }


  MainWindow window(&manager);
  window.show();

  int iRetVal = app.exec();


  ESerializingError se = manager.serializeTo(spTextWriter.get());
  assert(ESerializingError::eOk == se);


//  tspSerializer spMdWriter = SerializerFactory::create("markdown");
//  if (nullptr == spMdWriter ||
//      !spMdWriter->setParameter("fileName", "serialisation_markdown.txt"))
//  {
//    assert(false);
//  }
//  se = manager.serializeTo(spMdWriter.get());
//  assert(ESerializingError::eOk == se);


  return iRetVal;

}
