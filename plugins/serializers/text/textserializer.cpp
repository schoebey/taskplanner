#include "textserializer.h"
#include "serializerregister.h"
#include "serializablemanager.h"
#include "group.h"
#include "task.h"
#include "compat.h"

#include <QString>
#include <QVariant>
#include <QFile>

namespace
{
  RegisterSerializer<TextSerializer> s("text", "txt");

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
      m_stream << QString("task planner") << Qt::endl;
      m_stream << QDateTime::currentDateTime().toString(c_sTimeFormat) << Qt::endl;
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

      /*QString sHeader = */m_stream.readLine();
      /*QString sWriteTimestamp = */m_stream.readLine();

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
  m_stream << c_sManagerHeader << Qt::endl;
  m_stream << m.version() << Qt::endl;
  m_stream << m.groupIds().size() << Qt::endl;
  m_stream << m.taskIds().size() << Qt::endl;


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
    return EDeserializingError::eWrongFormat;
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
      task_id oldId = pTask->id();
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
  m_stream << c_sTaskHeader << Qt::endl;
  m_stream << t.version() << Qt::endl;
  m_stream << int(t.id()) << Qt::endl;
  m_stream << t.name() << Qt::endl;
  m_stream << t.description() << Qt::endl;

  m_stream << t.timeFragments().size() << Qt::endl;
  for (const auto& fragment : t.timeFragments())
  {
    m_stream << fragment.startTime.toString(c_sTimeFormat) << Qt::endl;
    m_stream << fragment.stopTime.toString(c_sTimeFormat) << Qt::endl;
  }

//  m_stream << t.priority();

  m_stream << int(t.parentTask()) << Qt::endl;

  m_stream << t.taskIds().size() << Qt::endl;
  for (const auto& id : t.taskIds())
  {
    m_stream << int(id) << Qt::endl;
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
    int iId;
    m_stream >> iId;

    m_stream.readLine();

    t.setId(iId);

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

    int iParentId;
    m_stream >> iParentId;
    m_stream.readLine();

    t.setParentTaskId(iParentId);

    int iNofSubTasks = 0;
    m_stream >> iNofSubTasks;
    m_stream.readLine();

    for (int i = 0; i < iNofSubTasks; ++i)
    {
      int iId;
      m_stream >> iId;
      m_stream.readLine();

      t.addTaskId(iId);
    }

    // TODO: deserialize generic properties

    return EDeserializingError::eOk;
  }

  return EDeserializingError::eInternalError;
}

ESerializingError TextSerializer::serialize(const Group& g)
{
  m_stream << c_sGroupHeader << Qt::endl;
  m_stream << g.version() << Qt::endl;
  m_stream << int(g.id()) << Qt::endl;
  m_stream << g.name() << Qt::endl;
  m_stream << g.description() << Qt::endl;

  m_stream << g.taskIds().size() << Qt::endl;
  for (const auto& id : g.taskIds())
  {
    m_stream << int(id) << Qt::endl;
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
    int iId;
    m_stream >> iId;
    m_stream.readLine();

    g.setId(iId);

    QString sName = m_stream.readLine();
    g.setName(sName);

    QString sDesc = m_stream.readLine();
    g.setDescription(sDesc);

    int iNofSubTasks = 0;
    m_stream >> iNofSubTasks;
    m_stream.readLine();

    for (int i = 0; i < iNofSubTasks; ++i)
    {
      int iId;
      m_stream >> iId;
      m_stream.readLine();

      g.addTaskId(iId);
    }

    return EDeserializingError::eOk;
  }

  return EDeserializingError::eInternalError;
}

