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
    if (m_file.open(QIODevice::ReadWrite))
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
    if (m_file.open(QIODevice::ReadOnly))
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
  m_stream << m.version();
  return ESerializingError::eOk;
}

EDeserializingError TextSerializer::deserialize(SerializableManager& m)
{
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
      group_id id;
      m_stream >> id;
      ISerializable* pGroup = m.addGroup(id);
      pGroup->deserialize(this);
    }


    for (int iTask = 0; iTask < iNofTasks; ++iTask)
    {
      task_id id;
      m_stream >> id;
      ISerializable* pTask = m.addTask(id);
      pTask->deserialize(this);
    }
  }

  return EDeserializingError::eOk;
}

ESerializingError TextSerializer::serialize(const Task&)
{
  return ESerializingError::eOk;
}

EDeserializingError TextSerializer::deserialize(Task&)
{
  return EDeserializingError::eOk;
}

ESerializingError TextSerializer::serialize(const Group&)
{
  return ESerializingError::eOk;
}

EDeserializingError TextSerializer::deserialize(Group&)
{
  return EDeserializingError::eOk;
}

