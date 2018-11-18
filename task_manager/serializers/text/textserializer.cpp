#include "textserializer.h"
#include "register.h"
#include "serializablemanager.h"
#include "group.h"
#include "task.h"

#include <QString>
#include <QVariant>
#include <QFile>

namespace
{
  Register<TextSerializer> s("text");

  static const QString c_sPara_FileName = "fileName";
}

TextSerializer::TextSerializer()
{
  registerParameter(c_sPara_FileName, QVariant::String, true);
}


ESerializingError TextSerializer::initSerialization()
{
  if (hasParameter(c_sPara_FileName))
  {
    QString sFileName = parameter(c_sPara_FileName).toString();
    m_file.setFileName(sFileName);
    if (m_file.open(QIODevice::ReadWrite | QIODevice::Text))
    {
      m_stream.setDevice(&m_file);
      m_stream.setCodec("UTF-8");
      m_stream << "task planner" << endl;
      m_stream << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz") << endl;;
      return ESerializingError::eOk;
    }

    return ESerializingError::eResourceError;
  }
  else
  {
    return ESerializingError::eWrongParameter;
  }
}

ESerializingError TextSerializer::deinitSerialization()
{
  m_stream.flush();
  m_file.close();
  return ESerializingError::eOk;
}


EDeserializingError TextSerializer::initDeserialization()
{
  if (hasParameter(c_sPara_FileName))
  {
    QString sFileName = parameter(c_sPara_FileName).toString();
    m_file.setFileName(sFileName);
    if (m_file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      m_stream.setDevice(&m_file);
      m_stream.setCodec("UTF-8");
      return EDeserializingError::eOk;
    }

    return EDeserializingError::eResourceError;
  }
  else
  {
    return EDeserializingError::eWrongParameter;
  }
}

EDeserializingError TextSerializer::deinitDeserialization()
{
  m_file.close();
  return EDeserializingError::eOk;
}

ESerializingError TextSerializer::serialize(const SerializableManager& m)
{
  m_stream << "\nmanager\n";
  m_stream << m.version();
  m_stream << m.groupIds().size();
  m_stream << m.taskIds().size();


  for (const auto & id : m.groupIds())
  {
    Group* pGroup = m.group(id);
    if (nullptr != pGroup)
    {
      serialize(*pGroup);
    }
  }

  for (const auto & id : m.taskIds())
  {
    Task* pTask = m.task(id);
    if (nullptr != pTask)
    {
      serialize(*pTask);
    }
  }


  return ESerializingError::eOk;
}

EDeserializingError TextSerializer::deserialize(SerializableManager& m)
{
  QString sHeader;
  m_stream >> sHeader;

  int iVersion = 0;
  m_stream >> iVersion;
  if (0 == iVersion)
  {
    int iNofGroups = 0;
    m_stream >> iNofGroups;
    int iNofTasks = 0;
    m_stream >> iNofTasks;

    for (int iGroup = 0; iGroup < iNofGroups; ++iGroup)
    {
      ISerializable* pGroup = m.addGroup();
      pGroup->deserialize(this);
    }


    for (int iTask = 0; iTask < iNofTasks; ++iTask)
    {
      ISerializable* pTask = m.addTask();
      pTask->deserialize(this);
    }
  }

  return EDeserializingError::eOk;
}

ESerializingError TextSerializer::serialize(const Task& t)
{
  m_stream << "\ntask\n";

  m_stream << t.version();
  m_stream << t.id();
  m_stream << t.name();
  m_stream << t.description();
  m_stream << t.parentTask();

  m_stream << t.taskIds().size();
  for (const auto& id : t.taskIds())
  {
    m_stream << id;
  }

  return ESerializingError::eOk;
}

EDeserializingError TextSerializer::deserialize(Task& t)
{
  QString sHeader;
  m_stream >> sHeader;

  int iVersion = 0;
  m_stream >> iVersion;

  if (0 == iVersion)
  {
    task_id id;
    m_stream >> id;
    t.setId(id);

    QString sName;
    m_stream >> sName;
    t.setName(sName);

    QString sDesc;
    m_stream >> sDesc;
    t.setDescription(sDesc);

    task_id parentId;
    m_stream >> parentId;
    t.setParentTaskId(parentId);

    int iNofSubTasks = 0;
    m_stream >> iNofSubTasks;
    for (int i = 0; i < iNofSubTasks; ++i)
    {
      task_id id;
      m_stream >> id;
      t.addTaskId(id);
    }

    return EDeserializingError::eOk;
  }

  return EDeserializingError::eInternalError;
}

ESerializingError TextSerializer::serialize(const Group& g)
{
  m_stream << "\ngroup\n";
  m_stream << g.version();
  m_stream << g.id();
  m_stream << g.name();
  m_stream << g.description();

  m_stream << g.taskIds().size();
  for (const auto& id : g.taskIds())
  {
    m_stream << id;
  }

  return ESerializingError::eOk;
}

EDeserializingError TextSerializer::deserialize(Group& g)
{
  QString sHeader;
  m_stream >> sHeader;

  int iVersion = 0;
  m_stream >> iVersion;

  if (0 == iVersion)
  {
    task_id id;
    m_stream >> id;
    g.setId(id);

    QString sName;
    m_stream >> sName;
    g.setName(sName);

    QString sDesc;
    m_stream >> sDesc;
    g.setDescription(sDesc);

    int iNofSubTasks = 0;
    m_stream >> iNofSubTasks;
    for (int i = 0; i < iNofSubTasks; ++i)
    {
      task_id id;
      m_stream >> id;
      g.addTaskId(id);
    }

    return EDeserializingError::eOk;
  }

  return EDeserializingError::eInternalError;
}

