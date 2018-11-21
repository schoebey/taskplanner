#include <QApplication>

#include "mainwindow.h"
#include "manager.h"
#include "task.h"
#include "group.h"
#include "serializerfactory.h"
#include "style.h"

#include <cassert>

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  app.setStyle(new Style());

  Manager manager;


  tspSerializer spTextWriter = SerializerFactory::create("text");
  if (!spTextWriter->setParameter("fileName", "serialisation_test.txt"))
  {
    assert(false);
  }

  EDeserializingError de = manager.deserializeFrom(spTextWriter.get());
  assert(EDeserializingError::eOk == de);


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

  int iRetVal = app.exec();


  ESerializingError se = manager.serializeTo(spTextWriter.get());
  assert(ESerializingError::eOk == se);


  tspSerializer spMdWriter = SerializerFactory::create("markdown");
  if (nullptr == spMdWriter ||
      !spMdWriter->setParameter("fileName", "serialisation_markdown.txt"))
  {
    assert(false);
  }
  se = manager.serializeTo(spMdWriter.get());
  assert(ESerializingError::eOk == se);


  return iRetVal;

}
