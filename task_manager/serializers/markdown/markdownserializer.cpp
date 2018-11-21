#include "markdownserializer.h"
#include "register.h"
#include "serializablemanager.h"
#include "group.h"
#include "task.h"

#include <QString>
#include <QVariant>
#include <QFile>

namespace
{
  Register<MarkdownSerializer> s("markdown");

  static const QString c_sPara_FileName = "fileName";
  static const QString c_sManagerHeader = "== manager ==";
  static const QString c_sTaskHeader = "== task ==";
  static const QString c_sGroupHeader = "== group ==";
  static const QString c_sTimeFormat = "yyyy-MM-dd hh:mm:ss.zzz";

  template<typename T> QString convert(const T& t);

  template<> QString convert(const int& i)
  {
    return QString::number(i);
  }

  template<> QString convert(const STimeFragment& f)
  {
    return QString("[%1|%2]").arg(f.startTime.toString(c_sTimeFormat)).arg(f.stopTime.toString(c_sTimeFormat));
  }

  template<typename T> T convert(const QString& s);

  template<> int convert(const QString& s)
  {
    bool bOk(false);
    int iRes = s.toInt(&bOk);
    return bOk ? iRes : 0;
  }

  template<> STimeFragment convert(const QString& s)
  {
    STimeFragment tf;

    QStringList parts = s.split("|");
    if (2 <= parts.size())
    {
      tf.startTime = QDateTime::fromString(parts[0], c_sTimeFormat);
      tf.stopTime = QDateTime::fromString(parts[1], c_sTimeFormat);
    }

    return tf;
  }


  template<typename T>
  void writeContainer(QTextStream& stream, const T& container, const QString& sName)
  {
    stream << sName << ":";
    QStringList list;
    for (const auto& el : container)
    {
      list.push_back(convert<typename T::value_type>(el));
    }
    stream << list.join(",");
    stream << endl;
  }

  template<typename T>
  bool readContainer(QTextStream& stream, T& container, const QString& sName)
  {
    QString sLine = stream.readLine();
    QStringList list = sLine.split(":");
    if (1 > list.size())  { return false; }


    QString sIdentifier = list[0];
    if (sIdentifier != sName)  { return false; }

    QStringList elements = list[2].split(",");

    for (const auto& el : elements)
    {
      container.push_back(convert<typename T::value_type>(el));
    }

    return true;
  }
}

MarkdownSerializer::MarkdownSerializer()
{
  registerParameter(c_sPara_FileName, QVariant::String, true);
}


ESerializingError MarkdownSerializer::initSerialization()
{
  if (hasParameter(c_sPara_FileName))
  {
    QString sFileName = parameter(c_sPara_FileName).toString();
    m_file.setFileName(sFileName);
    if (m_file.open(QIODevice::ReadWrite | QIODevice::Text))
    {
      m_stream.setDevice(&m_file);
      m_stream.setCodec("UTF-8");
      m_stream << QString("=== task planner ===") << endl;
      m_stream << "change date: " << QDateTime::currentDateTime().toString(c_sTimeFormat) << endl;
      return ESerializingError::eOk;
    }

    return ESerializingError::eResourceError;
  }
  else
  {
    return ESerializingError::eWrongParameter;
  }
}

ESerializingError MarkdownSerializer::deinitSerialization()
{
  m_stream.flush();
  m_file.close();
  return ESerializingError::eOk;
}


EDeserializingError MarkdownSerializer::initDeserialization()
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

EDeserializingError MarkdownSerializer::deinitDeserialization()
{
  m_file.close();
  return EDeserializingError::eOk;
}

ESerializingError MarkdownSerializer::serialize(const SerializableManager& m)
{
  m_stream << c_sManagerHeader << endl;
  m_stream << "version:" << m.version() << endl;

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

EDeserializingError MarkdownSerializer::deserialize(SerializableManager& m)
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
    while (!m_stream.atEnd())
    {
      qint64 pos = m_stream.pos();
      QString sIdentifier = m_stream.readLine();
      if (c_sGroupHeader == sIdentifier)
      {
        m_stream.seek(pos);
        Group* pGroup = m.addGroup();
        group_id oldId = pGroup->id();
        EDeserializingError err = pGroup->deserialize(this);
        m.changeGroupId(oldId, pGroup->id());
        if (EDeserializingError::eOk != err)  { return err; }
      }
      else if (c_sTaskHeader == sIdentifier)
      {
        m_stream.seek(pos);
        Task* pTask = m.addTask();
        group_id oldId = pTask->id();
        EDeserializingError err = pTask->deserialize(this);
        m.changeTaskId(oldId, pTask->id());
        if (EDeserializingError::eOk != err)  { return err; }
      }
    }
  }

  return EDeserializingError::eOk;
}

ESerializingError MarkdownSerializer::serialize(const Task& t)
{
  m_stream << c_sTaskHeader << endl;
  m_stream << "version:" << t.version() << endl;
  m_stream << "id:" << t.id() << endl;
  m_stream << "name:" << t.name() << endl;
  m_stream << "description:" << t.description() << endl;

  writeContainer(m_stream, t.timeFragments(), "time_info");

//  m_stream << t.priority();

  m_stream << "parent:" << t.parentTask() << endl;

  writeContainer(m_stream, t.taskIds(), "children");

  return ESerializingError::eOk;
}

EDeserializingError MarkdownSerializer::deserialize(Task& t)
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
    if (readContainer(m_stream, vTimeFragments, "time_info"))
    {
      t.setTimeFragments(vTimeFragments);
    }

    task_id parentId;
    m_stream >> parentId;
    m_stream.readLine();

    t.setParentTaskId(parentId);

    std::set<task_id> children;
    //readContainer(m_stream, children, "children");

    return EDeserializingError::eOk;
  }

  return EDeserializingError::eInternalError;
}

ESerializingError MarkdownSerializer::serialize(const Group& g)
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

EDeserializingError MarkdownSerializer::deserialize(Group& g)
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

