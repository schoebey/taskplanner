#ifndef GTEST_HELPERS_H
#define GTEST_HELPERS_H

#include <QString>
#include <QDateTime>
#include <ostream>

void PrintTo(const QString& s, ::std::ostream* os) {
  *os << s.toUtf8().constData();
}

void PrintTo(const QDate& d, ::std::ostream* os) {
  *os << d.toString("yyyy-MM-dd").toUtf8().constData();
}

void PrintTo(const QTime& t, ::std::ostream* os) {
  *os << t.toString("hh:mm:ss.zzz").toUtf8().constData();
}

void PrintTo(const QDateTime& dt, ::std::ostream* os) {
  *os << dt.toString("yyyy-MM-dd hh:mm:ss.zzz").toUtf8().constData();
}


#endif // GTEST_HELPERS_H
