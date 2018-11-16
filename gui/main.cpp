#include <QApplication>

#include "mainwindow.h"
#include "manager.h"
#include "task.h"
#include "group.h"
#include "serializerfactory.h"

#include <cassert>

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);


  Manager manager;

  IGroup* pGroup = manager.addGroup();
  pGroup->setName("testGroup");

  for (int i = 0; i < 5; ++i)
  {
    ITask* pTask = manager.addTask();
    pTask->setName(QString("test_%1").arg(i));
    pGroup->addTask(pTask->id());
  }

  pGroup = manager.addGroup();
  pGroup->setName("testGroup2");

  pGroup = manager.addGroup();
  pGroup->setName("testGroup3");


  tspSerializer spTextWriter = SerializerFactory::create("text");
  if (!spTextWriter->setParameter("fileName", "serialisation_test.txt"))
  {
    assert(false);
  }
  ESerializingError err = manager.serializeTo(spTextWriter.get());
  assert(ESerializingError::eOk == err);



  MainWindow window(&manager);
  window.show();

  return app.exec();
}
