#include "markdownserializer.h"
#include "serializerregister.h"
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
  RegisterSerializer<MarkdownSerializer> s("markdown", "md");

  static const QString c_sPara_FileName = "fileName";
  static const QString c_sManagerHeader = "## manager";
  static const QString c_sTaskHeader = "### task";
  static const QString c_sGroupHeader = "### group";
  static const QString c_sPropertyHeader = "#### property";
  static const QString c_sConstraintHeader = "##### constraint";
  static const QString c_sTimeFormat = "yyyy-MM-dd hh:mm:ss.zzz";

  QString startTagFromString(const QString& s)
  {
    if (!s.isEmpty())
    {
      QRegExp rx("^#+");
      if (0 == rx.indexIn(s))
      {
        return rx.cap(0);
      }
    }

    return QString();
  }

  class StreamSwapper
  {
  public:
    StreamSwapper(QTextStream** ppStream, const QString& sHeader)
      : m_ppStream(ppStream),
        m_sHeader(sHeader)
    { }

    virtual ~StreamSwapper()
    { }
  protected:
    QString m_string;
    QTextStream m_stream;
    QTextStream** m_ppStream;
    QTextStream* m_pOriginalStream;
    QString m_sHeader;
  };

  class StreamWriter : public StreamSwapper
  {
  public:
    StreamWriter(QTextStream** ppStream, const QString& sHeader)
      : StreamSwapper(ppStream, sHeader)
    {
      m_stream.setString(&m_string);
      m_pOriginalStream = *m_ppStream;
      *m_ppStream = &m_stream;
    }

    ~StreamWriter() override
    {
      m_stream.flush();
      *m_ppStream = m_pOriginalStream;
      **m_ppStream << m_sHeader << endl;
      **m_ppStream << m_string;
    }
  };

  class StreamReader : public StreamSwapper
  {
  public:
    StreamReader(QTextStream** ppStream, const QString& sHeader,
                 const QString& sContents = QString())
      : StreamSwapper(ppStream, sHeader)
    {
      if (sContents.isEmpty())
      {
        QString sStreamHeader = (*ppStream)->readLine();
        if (sStreamHeader.startsWith(sHeader))
        {
          QString sStartTag = startTagFromString(sStreamHeader);
          QString sChildStartTag = sStartTag + "#";

          while (!(*ppStream)->atEnd())
          {
            qint64 iLastValidPos = (*ppStream)->pos();
            QString sLine = (*ppStream)->readLine();
            if (sLine.startsWith(sChildStartTag) ||
                !sLine.startsWith("#"))
            {
              m_string += "\n" + sLine;
            }
            else
            {
              (*ppStream)->seek(iLastValidPos);
              break;
            }
          }

          m_bStatus = true;
        }
      }
      else
      {
        if (sContents.startsWith(sHeader))
        {
          m_string = sContents.right(sContents.size() - sContents.indexOf("\n") - 1);
          m_bStatus = true;
        }
      }

      if (m_bStatus)
      {
        m_stream.setString(&m_string);
        m_pOriginalStream = *m_ppStream;
        *m_ppStream = &m_stream;
      }
    }

    ~StreamReader() override
    {
      *m_ppStream = m_pOriginalStream;
    }

    operator bool() const
    {
      return m_bStatus;
    }

  private:
    bool m_bStatus = false;
  };

  template<typename T> struct is_container : std::false_type {};
  template<typename T> struct is_container<std::vector<T>> : std::true_type {};
  template<typename T> struct is_container<std::set<T>> : std::true_type {};

  template<typename T> QString convertFrom(const T& t);

  template<> QString convertFrom(const int& i)
  {
    return QString::number(i);
  }

  template<> QString convertFrom(const bool& b)
  {
    return QString(b ? "true" : "false");
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
  bool readFromMap(const std::map<QString, std::vector<QString>>& values,
                   const QString& sName, T& t, size_t index = 0)
  {

    auto it = values.find(sName);
    if (it == values.end())  { return false; }

    if (index < it->second.size() && !it->second.at(index).isEmpty())
    {
      t = convertTo<T>(it->second.at(index));
      return true;
    }

    return false;
  }

  std::map<QString, std::vector<QString>> valuesFromStream(QTextStream& stream)
  {
    qint64 iLastValidPos = 0;
    QString sLine;
    QString sData;
    std::map<QString, std::vector<QString>> values;
    while (!stream.atEnd())
    {
      sLine = stream.readLine();

      QString sStartSequence = startTagFromString(sLine);

      int idx = sLine.indexOf(":");
      QString sTag = sLine.left(idx);
      QString sData;
      if (sStartSequence.isEmpty())
      {
        // separate tag from value, store it
        sData = sLine.right(sLine.size() - idx - 1);
      }
      else
      {
        // read every line that has either no start sequence or
        // a start sequence that indicates a child of the current
        // element.
        // separate tag from size if available, then keep reading
        // until another line with a higher priority start tag is read
        sData = sLine;
        QString sChildStartTag = sStartSequence + "#";
        while (!stream.atEnd())
        {
          // remember this position to be later able to jump back to it
          // if content has been read that doesn't belong to the current section.
          iLastValidPos = stream.pos();

          sLine = stream.readLine();
          if (sLine.startsWith(sChildStartTag) ||
              !sLine.startsWith("#"))
          {
            sData += "\n" + sLine;
          }
          else
          {
            stream.seek(iLastValidPos);
            break;
          }
        }
      }

      values[sTag].push_back(sData);
    }

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
    if (m_file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate))
    {
      if (nullptr == m_pStream)
      {
        m_pStream = new QTextStream();
      }
      m_pStream->setDevice(&m_file);
      m_pStream->setCodec("UTF-8");
      *m_pStream << QString("# task planner") << endl;
      *m_pStream << "change date: " << QDateTime::currentDateTime().toString(c_sTimeFormat) << endl;
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
  m_pStream->flush();
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
      if (nullptr == m_pStream)
      {
        m_pStream = new QTextStream();
      }
      m_pStream->setDevice(&m_file);
      m_pStream->setCodec("UTF-8");

      QString sHeader = m_pStream->readLine();
      QString sWriteTimestamp = m_pStream->readLine();

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
  StreamWriter s(&m_pStream, c_sManagerHeader);

  writeToStream(*m_pStream, m.version(), "version");

  for (const QString& sName : Properties::registeredPropertyNames())
  {
    tspDescriptor spDescriptor = Properties::descriptor(sName);
    spDescriptor->serialize(this);
  }

  for (const auto & id : m.groupIds())
  {
    Group* pGroup = m.group(id);
    if (nullptr != pGroup)
    {
      pGroup->serialize(this);
    }
  }

  for (const auto & id : m.taskIds())
  {
    Task* pTask = m.task(id);
    if (nullptr != pTask)
    {
      pTask->serialize(this);
    }
  }

  return ESerializingError::eOk;
}

EDeserializingError MarkdownSerializer::deserialize(SerializableManager& m)
{
  StreamReader r(&m_pStream, c_sManagerHeader);
  if (!r)  { return EDeserializingError::eInternalError; }

  std::map<QString, std::vector<QString>> values = valuesFromStream(*m_pStream);
  int iVersion = 0;
  if (!readFromMap(values, "version", iVersion))
  {
    assert(false && "couldn't read version from stream");
    return EDeserializingError::eInternalError;
  }

  if (0 == iVersion)
  {
    // property descriptors
    QString sPayload;
    size_t index(0);
    while (readFromMap(values, c_sPropertyHeader, sPayload, index++))
    {
      StreamReader p(&m_pStream, c_sPropertyHeader, sPayload);

      std::map<QString, std::vector<QString>> propertyDescriptionValues = valuesFromStream(*m_pStream);

      QString sName;
      tspDescriptor spDescriptor;
      if (readFromMap(propertyDescriptionValues, "name", sName)&&
          nullptr != (spDescriptor = Properties::descriptor(sName)))
      {
        StreamReader p2(&m_pStream, c_sPropertyHeader, sPayload);
        EDeserializingError err = spDescriptor->deserialize(this);
        if (EDeserializingError::eOk != err)  { return err; }
      }
    }

    // groups
    index = 0;
    while (readFromMap(values, c_sGroupHeader, sPayload, index++))
    {
      Group* pGroup = m.addGroup();
      group_id oldId = pGroup->id();

      StreamReader g(&m_pStream, c_sGroupHeader, sPayload);
      EDeserializingError err = pGroup->deserialize(this);
      m.changeGroupId(oldId, pGroup->id());
      if (EDeserializingError::eOk != err)  { return err; }
    }

    // tasks
    index = 0;
    while (readFromMap(values, c_sTaskHeader, sPayload, index++))
    {
      Task* pTask = m.addTask();
      group_id oldId = pTask->id();

      StreamReader t(&m_pStream, c_sTaskHeader, sPayload);
      EDeserializingError err = pTask->deserialize(this);
      m.changeTaskId(oldId, pTask->id());
      if (EDeserializingError::eOk != err)  { return err; }
    }
  }

  return EDeserializingError::eOk;
}

ESerializingError MarkdownSerializer::serialize(const PropertyDescriptor& descriptor)
{
  StreamWriter s(&m_pStream, c_sPropertyHeader);

  writeToStream(*m_pStream, descriptor.version(), "version");
  writeToStream(*m_pStream, descriptor.name(), "name");
  writeToStream(*m_pStream, descriptor.typeName(), "typeName");
  writeToStream(*m_pStream, descriptor.visible(), "visible");

  if (nullptr != descriptor.constraint())
  {
    descriptor.constraint()->serialize(this);
  }

  return ESerializingError::eOk;
}

EDeserializingError MarkdownSerializer::deserialize(PropertyDescriptor& descriptor)
{
  std::map<QString, std::vector<QString>> values = valuesFromStream(*m_pStream);

  QString sPayload;
  if (readFromMap(values, c_sConstraintHeader, sPayload))
  {
    StreamReader c(&m_pStream, c_sConstraintHeader, sPayload);

    std::map<QString, std::vector<QString>> constraintValues = valuesFromStream(*m_pStream);
    QString sType;
    QString sContent;
    if (readFromMap(constraintValues, "type", sType) &&
        readFromMap(constraintValues, "content", sContent))
    {
      descriptor.addConstraint(sType, sContent);
    }

    // TODO: Read all constraint properties, call factory method to create
    // a new constraint 'name' with content 'content'
    // constraintfactory::create(sName, sContent);
    // descriptor.setConstraint(constraint);
  }

  return EDeserializingError::eOk;
}

ESerializingError MarkdownSerializer::serialize(const IConstraint& constraint)
{
  StreamWriter s(&m_pStream, c_sConstraintHeader);

  writeToStream(*m_pStream, constraint.version(), "version");
  writeToStream(*m_pStream, constraint.name(), "type");
  writeToStream(*m_pStream, constraint.toString(), "content");

  return ESerializingError::eOk;
}

EDeserializingError MarkdownSerializer::deserialize(IConstraint& /*constraint*/)
{
  // this should never be called, since constraints don't deserialize themselves...
  return EDeserializingError::eInternalError;
}

ESerializingError MarkdownSerializer::serialize(const Task& t)
{
  StreamWriter s(&m_pStream, c_sTaskHeader);

  writeToStream(*m_pStream, t.version(), "version");
  writeToStream(*m_pStream, t.id(), "id");

  for (const auto& name : t.propertyNames())
  {
    writeToStream(*m_pStream, t.propertyValue(name), name);
  }

  writeToStream(*m_pStream, t.timeFragments(), "time_info");
  writeToStream(*m_pStream, t.priority(), "priority");
  writeToStream(*m_pStream, t.parentTask(), "parent");
  writeToStream(*m_pStream, t.taskIds(), "children");

  return ESerializingError::eOk;
}

EDeserializingError MarkdownSerializer::deserialize(Task& t)
{
  std::map<QString, std::vector<QString>> values = valuesFromStream(*m_pStream);


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

    for (const auto& name : Properties::registeredPropertyNames())
    {
      QString sPropertyValue;
      if (readFromMap(values, name, sPropertyValue))
      {
        t.setPropertyValue(name, sPropertyValue);
      }
    }

    return EDeserializingError::eOk;
  }

  return EDeserializingError::eInternalError;
}

ESerializingError MarkdownSerializer::serialize(const Group& g)
{
  StreamWriter s(&m_pStream, c_sGroupHeader);

  writeToStream(*m_pStream, g.version(), "version");
  writeToStream(*m_pStream, g.id(), "id");
  writeToStream(*m_pStream, g.name(), "name");
  writeToStream(*m_pStream, g.description(), "description");
  writeToStream(*m_pStream, g.taskIds(), "tasks");

  return ESerializingError::eOk;
}

EDeserializingError MarkdownSerializer::deserialize(Group& g)
{
  std::map<QString, std::vector<QString>> values = valuesFromStream(*m_pStream);
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

