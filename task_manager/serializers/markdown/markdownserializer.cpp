#include "markdownserializer.h"
#include "register.h"
#include "serializablemanager.h"
#include "group.h"
#include "task.h"
#include "container_utils.h"

#include <QString>
#include <QVariant>
#include <QFile>
#include <QStringList>

#include <cassert>

namespace
{
  Register<MarkdownSerializer> s("markdown");

  static const QString c_sPara_FileName = "fileName";
  static const QString c_sManagerHeader = "== manager ==";
  static const QString c_sTaskHeader = "== task ==";
  static const QString c_sGroupHeader = "== group ==";
  static const QString c_sTimeFormat = "yyyy-MM-dd hh:mm:ss.zzz";

  template<typename T> struct is_container : std::false_type {};
  template<typename T> struct is_container<std::vector<T>> : std::true_type {};
  template<typename T> struct is_container<std::set<T>> : std::true_type {};

  template<typename T> QString convertFrom(const T& t);

  template<> QString convertFrom(const int& i)
  {
    return QString::number(i);
  }

  template<> QString convertFrom(const STimeFragment& f)
  {
    QDateTime stopTime = f.stopTime.isValid() ? f.stopTime : QDateTime::currentDateTime();
    return QString("[%1|%2]").arg(f.startTime.toString(c_sTimeFormat)).arg(stopTime.toString(c_sTimeFormat));
  }

  template<> QString convertFrom(const SPriority& prio)
  {
    return convertFrom(prio.categories);
  }

  template<> QString convertFrom(const QString& s)
  {
    return s;
  }

  template<typename T>
  QString convertFrom(const T& container)
  {
    QStringList list;
    for (const auto& el : container)
    {
      list.push_back(convertFrom<typename T::value_type>(el));
    }
    return list.join(",");
  }

  template<typename T>
  typename std::enable_if<!is_container<T>::value, T>::type
  convertTo(const QString& s);

  template<> int convertTo<int>(const QString& s)
  {
    bool bOk(false);
    int iRes = s.toInt(&bOk);
    return bOk ? iRes : 0;
  }

  template<> QString convertTo<QString>(const QString& s)
  {
    return s;
  }

  template<> STimeFragment convertTo<STimeFragment>(const QString& s)
  {
    STimeFragment tf;

    QStringList parts = s.split("|");
    if (2 <= parts.size())
    {
      tf.startTime = QDateTime::fromString(parts[0].remove(0,1), c_sTimeFormat);
      tf.stopTime = QDateTime::fromString(parts[1].remove(parts[1].size() - 1, 1), c_sTimeFormat);
    }

    return tf;
  }

  template<typename T>
  typename std::enable_if<is_container<T>::value, T>::type
  convertTo(const QString& s)
  {
    T container;
    QStringList elements = s.split(",");

    for (const auto& el : elements)
    {
      container.insert(container.end(),
                       convertTo<typename T::value_type>(el));
    }

    return container;
  }

  template<> SPriority convertTo<SPriority>(const QString& sPrio)
  {
    SPriority prio;
    prio.categories = convertTo<decltype(prio.categories)>(sPrio);
    return prio;
  }

  template<typename T>
  void writeToStream(QTextStream& stream, const T& t, const QString& sName)
  {
    stream << sName << ":";
    stream << convertFrom(t);
    stream << endl;
  }

  template<typename T>
  bool readFromMap(const std::map<QString, QString>& values, const QString& sName, T& t)
  {

    auto it = values.find(sName);
    if (it == values.end())  { return false; }

    if (!it->second.isEmpty())
    {
      t = convertTo<T>(it->second);
      return true;
    }

    return false;
  }

  std::map<QString, QString> valuesFromStream(QTextStream& stream)
  {
    qint64 pos = stream.pos();
    QString sLine;
    std::map<QString, QString> values;
    while (!sLine.startsWith("==") && !stream.atEnd())
    {
      pos = stream.pos();
      sLine = stream.readLine();
      int idx = sLine.indexOf(":");
      values[sLine.left(idx)] = sLine.right(sLine.size() - idx - 1);
    }
    stream.seek(pos);
    return values;
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
  writeToStream(m_stream, m.version(), "version");

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

  std::map<QString, QString> values = valuesFromStream(m_stream);
  int iVersion = 0;
  if (!readFromMap(values, "version", iVersion))
  {
    assert(false && "couldn't read version from stream");
    return EDeserializingError::eInternalError;
  }

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
  writeToStream(m_stream, t.version(), "version");
  writeToStream(m_stream, t.id(), "id");
  writeToStream(m_stream, t.name(), "name");
  writeToStream(m_stream, t.description(), "description");
  writeToStream(m_stream, t.timeFragments(), "time_info");
  writeToStream(m_stream, t.priority(), "priority");
  writeToStream(m_stream, t.parentTask(), "parent");
  writeToStream(m_stream, t.taskIds(), "children");

  return ESerializingError::eOk;
}

EDeserializingError MarkdownSerializer::deserialize(Task& t)
{
  QString sHeader = m_stream.readLine();
  if (sHeader != c_sTaskHeader)
  {
    return EDeserializingError::eInternalError;
  }

  std::map<QString, QString> values = valuesFromStream(m_stream);


  int iVersion = 0;
  if (!readFromMap(values, "version", iVersion))
  {
    assert(false && "couldn't read version from stream");
    return EDeserializingError::eInternalError;
  }


  if (0 == iVersion)
  {
    task_id id;
    if (readFromMap(values, "id", id))
    {
      t.setId(id);
    }

    QString sName;
    if (readFromMap(values, "name", sName))
    {
      t.setName(sName);
    }

    QString sDesc;
    if (readFromMap(values, "description", sDesc))
    {
      t.setDescription(sDesc);
    }

    std::vector<STimeFragment> vTimeFragments;
    if (readFromMap(values, "time_info", vTimeFragments))
    {
      t.setTimeFragments(vTimeFragments);
    }

    SPriority prio;
    if (readFromMap(values, "priority", prio))
    {
      t.setPriority(prio);
    }

    task_id parentId;
    if (readFromMap(values, "parent", parentId))
    {
      t.setParentTaskId(parentId);
    }

    std::set<task_id> children;
    if (readFromMap(values, "children", children))
    {
      for (const auto& childId : children)
      {
        t.addTaskId(childId);
      }
    }

    return EDeserializingError::eOk;
  }

  return EDeserializingError::eInternalError;
}

ESerializingError MarkdownSerializer::serialize(const Group& g)
{
  m_stream << c_sGroupHeader << endl;
  writeToStream(m_stream, g.version(), "version");
  writeToStream(m_stream, g.id(), "id");
  writeToStream(m_stream, g.name(), "name");
  writeToStream(m_stream, g.description(), "description");
  writeToStream(m_stream, g.taskIds(), "tasks");

  return ESerializingError::eOk;
}

EDeserializingError MarkdownSerializer::deserialize(Group& g)
{
  QString sHeader = m_stream.readLine();
  if (sHeader != c_sGroupHeader)
  {
    return EDeserializingError::eInternalError;
  }


  std::map<QString, QString> values = valuesFromStream(m_stream);
  int iVersion = 0;
  if (!readFromMap(values, "version", iVersion))
  {
    assert(false && "couldn't read version from stream");
    return EDeserializingError::eInternalError;
  }


  if (0 == iVersion)
  {
    group_id id;
    if (readFromMap(values, "id", id))
    {
      g.setId(id);
    }

    QString sName;
    if (readFromMap(values, "name", sName))
    {
      g.setName(sName);
    }

    QString sDesc;
    if (readFromMap(values, "description", sDesc))
    {
      g.setDescription(sDesc);
    }

    std::set<task_id> children;
    if (readFromMap(values, "tasks", children))
    {
      for (const auto& childId : children)
      {
        g.addTaskId(childId);
      }
    }

    return EDeserializingError::eOk;
  }

  return EDeserializingError::eInternalError;
}

