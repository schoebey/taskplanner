#include <QApplication>

#include "mainwindow.h"
#include "manager.h"
#include "task.h"
#include "group.h"
#include "serializerfactory.h"
#include "style.h"
#include "property.h"

#include <QDateTime>

#include <cassert>


namespace conversion
{
}

int main(int argc, char *argv[])
{
  // test code for properties
  REGISTER_PROPERTY("name", QString);
  REGISTER_PROPERTY("due date", QDateTime);
//  Properties::registerProperty("name", "QString");
//  Properties::registerProperty("priority", "int");

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


  if (EDeserializingError::eOk != de | 0 == manager.groupIds().size())
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
