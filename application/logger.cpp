#include "logger.h"

#include <QFile>
#include <QTextStream>
#include <QDateTime>

namespace
{
  static const QString c_sDtFormat = "yyyy-MM-dd hh:mm:ss.zzz";
  static const QString c_sDelim = " - ";
}

Logger::Logger()
{

}

bool Logger::setFileName(const QString& sFileName)
{
  m_spFile = std::unique_ptr<QFile>(new QFile(sFileName));
  m_spStream = std::unique_ptr<QTextStream>(new QTextStream(m_spFile.get()));

  return m_spFile->open(QIODevice::ReadWrite | QIODevice::Append);
}

void Logger::log(const QString& sMsg)
{
  (*m_spStream.get()) << QDateTime::currentDateTime().toString(c_sDtFormat) << c_sDelim << sMsg << endl;
  m_spStream->flush();
}
