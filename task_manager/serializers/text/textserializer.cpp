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
  Register<TextSerializer> s("text", "txt");

  static const QString c_sPara_FileName = "fileName";
  static const QString c_sManagerHeader = "== manager ==";
  static const QString c_sTaskHeader = "== task ==";
  static const QString c_sGroupHeader = "== group ==";
  static const QString c_sTimeFormat = "yyyy-MM-dd hh:mm:ss.zzz";
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
      m_stream << QString("task planner") << endl;
      m_stream << QDateTime::currentDateTime().toString(c_sTimeFormat) << endl;
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

      QString sHeader = m_stream.readLine();
      QString sWriteTimestamp = m_stream.readLine();

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
  m_stream << c_sManagerHeader << endl;
  m_stream << m.version() << endl;
  m_stream << m.groupIds().size() << endl;
  m_stream << m.taskIds().size() << endl;


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
  QString sHeader = m_stream.readLine();
  if (sHeader != c_sManagerHeader)
  {
    return EDeserializingError::eInternalError;
  }

  int iVersion = 0;
  m_stream >> iVersion;
  m_stream.readLine();

  if (0 == iVersion)
  {
    int iNofGroups = 0;
    m_stream >> iNofGroups;
    m_stream.readLine();

    int iNofTasks = 0;
    m_stream >> iNofTasks;
    m_stream.readLine();

    for (int iGroup = 0; iGroup < iNofGroups; ++iGroup)
    {
      Group* pGroup = m.addGroup();
      group_id oldId = pGroup->id();
      EDeserializingError err = pGroup->deserialize(this);
      m.changeGroupId(oldId, pGroup->id());
      if (EDeserializingError::eOk != err)  { return err; }
    }


    for (int iTask = 0; iTask < iNofTasks; ++iTask)
    {
      Task* pTask = m.addTask();
      group_id oldId = pTask->id();
      EDeserializingError err = pTask->deserialize(this);
      m.changeTaskId(oldId, pTask->id());
      if (EDeserializingError::eOk != err)  { return err; }
    }
  }

  return EDeserializingError::eOk;
}

ESerializingError TextSerializer::serialize(const PropertyDescriptor&)
{
  return ESerializingError::eInternalError;
}

EDeserializingError TextSerializer::deserialize(PropertyDescriptor&)
{
  return EDeserializingError::eInternalError;
}

ESerializingError TextSerializer::serialize(const IConstraint&)
{
  return ESerializingError::eInternalError;
}

EDeserializingError TextSerializer::deserialize(IConstraint&)
{
  return EDeserializingError::eInternalError;
}

ESerializingError TextSerializer::serialize(const Task& t)
{
  m_stream << c_sTaskHeader << endl;
  m_stream << t.version() << endl;
  m_stream << t.id() << endl;
  m_stream << t.name() << endl;
  m_stream << t.description() << endl;

  m_stream << t.timeFragments().size() << endl;
  for (const auto& fragment : t.timeFragments())
  {
    m_stream << fragment.startTime.toString(c_sTimeFormat) << endl;
    m_stream << fragment.stopTime.toString(c_sTimeFormat) << endl;
  }

//  m_stream << t.priority();

  m_stream << t.parentTask() << endl;

  m_stream << t.taskIds().size() << endl;
  for (const auto& id : t.taskIds())
  {
    m_stream << id << endl;
  }

  // TODO: serialize generic properties

  return ESerializingError::eOk;
}

EDeserializingError TextSerializer::deserialize(Task& t)
{
  QString sHeader = m_stream.readLine();
  if (sHeader != c_sTaskHeader)
  {
    return EDeserializingError::eInternalError;
  }

  int iVersion = 0;
  m_stream >> iVersion;
  m_stream.readLine();


  if (0 == iVersion)
  {
    task_id id;
    m_stream >> id;
    m_stream.readLine();

    t.setId(id);

    QString sName = m_stream.readLine();
    t.setName(sName);

    QString sDesc = m_stream.readLine();
    t.setDescription(sDesc);

    int iNofTimeFragments = 0;
    m_stream >> iNofTimeFragments;
    m_stream.readLine();

    std::vector<STimeFragment> vTimeFragments;
    for (int i = 0; i < iNofTimeFragments; ++i)
    {
      QString sStartTime = m_stream.readLine();
      QString sStopTime = m_stream.readLine();

      STimeFragment fragment;
      fragment.startTime = QDateTime::fromString(sStartTime, c_sTimeFormat);
      fragment.stopTime = QDateTime::fromString(sStopTime, c_sTimeFormat);
      vTimeFragments.push_back(fragment);
    }
    t.setTimeFragments(vTimeFragments);

    task_id parentId;
    m_stream >> parentId;
    m_stream.readLine();

    t.setParentTaskId(parentId);

    int iNofSubTasks = 0;
    m_stream >> iNofSubTasks;
    m_stream.readLine();

    for (int i = 0; i < iNofSubTasks; ++i)
    {
      task_id id;
      m_stream >> id;
      m_stream.readLine();

      t.addTaskId(id);
    }

    // TODO: deserialize generic properties

    return EDeserializingError::eOk;
  }

  return EDeserializingError::eInternalError;
}

ESerializingError TextSerializer::serialize(const Group& g)
{
  m_stream << c_sGroupHeader << endl;
  m_stream << g.version() << endl;
  m_stream << g.id() << endl;
  m_stream << g.name() << endl;
  m_stream << g.description() << endl;

  m_stream << g.taskIds().size() << endl;
  for (const auto& id : g.taskIds())
  {
    m_stream << id << endl;
  }

  return ESerializingError::eOk;
}

EDeserializingError TextSerializer::deserialize(Group& g)
{
  QString sHeader = m_stream.readLine();
  if (sHeader != c_sGroupHeader)
  {
    return EDeserializingError::eInternalError;
  }

  int iVersion = 0;
  m_stream >> iVersion;
  m_stream.readLine();


  if (0 == iVersion)
  {
    task_id id;
    m_stream >> id;
    m_stream.readLine();

    g.setId(id);

    QString sName = m_stream.readLine();
    g.setName(sName);

    QString sDesc = m_stream.readLine();
    g.setDescription(sDesc);

    int iNofSubTasks = 0;
    m_stream >> iNofSubTasks;
    m_stream.readLine();

    for (int i = 0; i < iNofSubTasks; ++i)
    {
      task_id id;
      m_stream >> id;
      m_stream.readLine();

      g.addTaskId(id);
    }

    return EDeserializingError::eOk;
  }

  return EDeserializingError::eInternalError;
}

