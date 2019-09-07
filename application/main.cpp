#include <QCoreApplication>
#include <QApplication>

#include "mainwindow.h"
#include "manager.h"
#include "task.h"
#include "group.h"
#include "serializerfactory.h"
#include "style.h"
#include "property.h"
#include "constraint.h"
#include "constraint_grammar.h"
#include "propertyeditorfactory.h"
#include "editablelabel.h"
#include "taskwidget.h"
#include "version.h"

#include <QDateTime>
#include <QColor>

#include <QFileInfo>
#include <cassert>




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



  PropertyEditorFactory::registerEditor<EditableLabel, TaskWidget>("due date");

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

//  QCoreApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);
  QGuiApplication::setApplicationDisplayName(QString("%1 (%2)")
                                             .arg(taskplanner::project_name)
                                             .arg(taskplanner::project_full_version));


  Manager manager;



  MainWindow window(&manager);

  window.setStyle(new Style());
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

  // load last used file
  if (!bLoadedFileFromArgs &&!
      window.loadMostRecentFile())
  {
    window.restoreDefaultLayout();
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
