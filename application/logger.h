#ifndef LOGGER_H
#define LOGGER_H

#include <QString>

#include <memory>

class QFile;
class QTextStream;
class Logger
{
public:
  Logger();

  bool setFileName(const QString& sFileName);

  void log(const QString& sMsg);

private:
  std::unique_ptr<QFile> m_spFile;
  std::unique_ptr<QTextStream> m_spStream;
};

#endif // LOGGER_H
