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
#include <QColor>

#include <QFileInfo>
#include <cassert>

namespace grammar
{
  // logical
  #define AND &
  #define BUT AND
  #define OR |

  // constraints
  #define ONE_OF(a, ...) tspConstraintTpl<decltype(a)>(make_list<decltype(a)>(a, ## __VA_ARGS__))
  #define MIN(a) tspConstraintTpl<decltype(a)>(std::make_shared<MinConstraint<decltype(a)>>(a))
  #define MAX(a) tspConstraintTpl<decltype(a)>(std::make_shared<MaxConstraint<decltype(a)>>(a))
  #define EVEN_NUMBER std::make_shared<ConstraintImpl>()


  template<typename T> tspConstraintTpl<T> operator&(const std::shared_ptr<ConstraintTpl<T>>& p,
                                                  const std::shared_ptr<ConstraintTpl<T>>& p2)
  {
    return std::make_shared<AndConstraint<T>>(p, p2);
  }
  template<typename T> tspConstraintTpl<T> operator|(tspConstraintTpl<T> p, tspConstraintTpl<T> p2)
  {
    return std::make_shared<OrConstraint<T>>(p, p2);
  }

  // todo: not operator: inverts the rhs constraint
}



int main(int argc, char *argv[])
{
//  QRegExp relativeToNow("^in ((?:\\S*\\s*)*)\\s+(\\S+){1}$");
//  QString s("in a hundred years");
//  if (0 == relativeToNow.indexIn(s))
//  {
//    QString s1 = relativeToNow.cap(1);
//    QString s2 = relativeToNow.cap(2);
//    QString s3 = relativeToNow.cap(3);
//  }


//  bool bIsInt = false;
//  int iTest = conversion::stringToInt("ninehundredfiftytwo", &bIsInt);
//  assert(bIsInt && iTest == 952);

//  iTest = conversion::stringToInt("fiftyonethousandsevenhundredandfive", &bIsInt);
//  assert(bIsInt && iTest == 51705);

//  iTest = conversion::stringToInt("seventytwomillionandthree", &bIsInt);
//  assert(bIsInt && iTest == 72000003);

//  iTest = conversion::stringToInt("hello world", &bIsInt);
//  assert(!bIsInt);


//  // test code for constraints
//  using namespace grammar;

//  //auto a = ListConstraintTpl<int>(1,2,3,4,5,6);
//  //auto a = make_list(1,2,3,4,5);
////  auto b = ONE_OF(1,2,3);
//  auto sp = ONE_OF(1,2,3) OR MIN(10) AND MAX(20);
//  auto sp2 = ONE_OF(1.5,2.,3.) OR MIN(10.) AND MAX(20.);
//  auto sp25 = ONE_OF(QString("i"), QString("am"), QString("ready"));
//  auto sp3 = ONE_OF(QString("hello"), QString("world"), QString("abc")) OR ONE_OF(QString("i"), QString("am"), QString("ready"));

//  bool bOk = sp->accepts(12);
//  bOk = sp->accepts(8);
//  bOk = sp->accepts(22);
//  bOk = sp->accepts(1);
//  bOk = sp->accepts(2);
//  bOk = sp->accepts(3);
//  bOk = sp3->accepts("hello");
//  bOk = sp3->accepts("wrold");


//  tspConstraintTpl spResult = ONE_OF('a', 'b', 'c') OR (EVEN_NUMBER BUT MIN(0) AND MAX(100));


  // test code for properties
  REGISTER_PROPERTY(Task, "name", QString, false);
  REGISTER_PROPERTY(Task, "description", QString, false);
  REGISTER_PROPERTY(Task, "expanded", bool, false);
  REGISTER_PROPERTY(Task, "added date", QDateTime, false);
  REGISTER_PROPERTY(Task, "due date", QDateTime, true);
  REGISTER_PROPERTY(Task, "duration (days)", int, true);
  REGISTER_PROPERTY(Task, "category", QString, true);
  REGISTER_PROPERTY(Task, "links", std::vector<QUrl>, false);
  REGISTER_PROPERTY(Task, "sort_priority", int, false);
  REGISTER_PROPERTY(Task, "priority", int, true);
  REGISTER_PROPERTY(Task, "color", QColor, false);
  Properties<Task>::registerConstraint("category", ONE_OF(QString("a"), QString("b"), QString("c")));
  Properties<Task>::registerConstraint("duration (days)", MIN(0));

  REGISTER_PROPERTY(Group, "name", QString, false);
  REGISTER_PROPERTY(Group, "description", QString, false);
  REGISTER_PROPERTY(Group, "autoSorting", bool, false);


//  TODO: configure order of comparison for properties, e.g.:
//    (due date - expected duration), priority(high,med,low), category(feature,bugfix,refactoring,documentation)

//  Properties<Task> props;
//  props.set("due date", QDateTime::currentDateTime());
//  props.get<QDateTime>("due date");




//  tspSerializer spTextWriter = SerializerFactory::create("markdown");
//  if (!spTextWriter->setParameter("fileName", "serialisation_markdown.txt"))
//  {
//    assert(false);
//  }

//  EDeserializingError de = manager.deserializeFrom(spTextWriter.get());



  QApplication app(argc, argv);

  QString version(GIT_VERSION);
  QGuiApplication::setApplicationDisplayName(QString("TaskMänätscher (%1)").arg(version));

  app.setStyle(new Style());

  Manager manager;

  if (0 == manager.groupIds().size())
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

  bool bLoadedFileFromArgs = false;
  for (const QString& sArg : app.arguments())
  {
    QFileInfo info(sArg);
    if (info.exists() &&
        window.loadFile(info.fileName()))
    {
      bLoadedFileFromArgs = true;
      break;
    }
  }

  // load default file? get last loaded file from config?
  if (!bLoadedFileFromArgs)
  {
    window.loadFile("default.md");
  }


  int iRetVal = app.exec();


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
