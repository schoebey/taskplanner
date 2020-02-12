#ifndef GTEST_HELPERS_H
#define GTEST_HELPERS_H

#include <QString>
#include <ostream>

void PrintTo(const QString& s, ::std::ostream* os) {
  *os << s.toUtf8().constData();
}

#endif // GTEST_HELPERS_H
