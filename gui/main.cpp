#include <QApplication>

#include "mainwindow.h"
#include "manager.h"
#include "task.h"
#include "group.h"
#include "serializerfactory.h"
#include "style.h"
#include "property.h"

#include <QDateTime>

#include <cassert> test

namespace grammar
{
  // logical
  #define AND &
  #define BUT AND
  #define OR |

  // constraints
  #define ANY_OF(...) std::make_shared<ConstraintImpl>()
  #define MIN(a) std::make_shared<ConstraintImpl>()
  #define MAX(a) std::make_shared<ConstraintImpl>()
  #define EVEN_NUMBER std::make_shared<ConstraintImpl>()

  class Constraint
  {
  public:
    void blah() {}
  };
  typedef std::shared_ptr<Constraint> tspConstraint;

  class ConstraintImpl : public Constraint
  {
  public:
  };


  tspConstraint operator&(tspConstraint p, tspConstraint p2)
  {
    return std::make_shared<ConstraintImpl>();
  }
  tspConstraint operator|(tspConstraint p, tspConstraint p2)
  {
    return std::make_shared<ConstraintImpl>();
  }
}


int main(int argc, char *argv[])
{
  // test code for constraints
  using namespace grammar;

  tspConstraint spA;
  tspConstraint spA2;
  std::shared_ptr<ConstraintImpl> spB;

  tspConstraint spResult = ANY_OF('a', 'b', 'c') OR (EVEN_NUMBER BUT MIN(0) AND MAX(100));


  // test code for properties
  REGISTER_PROPERTY("name", QString, false);
  REGISTER_PROPERTY("description", QString, false);
  REGISTER_PROPERTY("expanded", bool, false);
  REGISTER_PROPERTY("due date", QDateTime, true);
  REGISTER_PROPERTY("due date time", QDateTime, true);

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
