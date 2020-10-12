#ifndef CONVERSION_FANCY_H
#define CONVERSION_FANCY_H

#include "libtaskmanager.h"

#include <QString>

class QTime;

namespace conversion
{

  template<typename T> QString toDisplayString(const T& val);

  static const QString c_sTimeFormat = "hh:mm:ss.zzz";
  static const QString c_sDateTimeFormat = "yyyy-MM-dd hh:mm:ss.zzz";

  namespace fancy
  {
    QString LIBTASKMANAGER toString(const QTime& t);

    QString LIBTASKMANAGER dateToString(const QString& sDate);

    QString LIBTASKMANAGER toString(int iValue, int iDepth = 0);

    int LIBTASKMANAGER toInt(const QString& sNumber, bool* pbStatus, int iStartValue = 0);
  }
}

#endif // CONVERSION_FANCY_H
