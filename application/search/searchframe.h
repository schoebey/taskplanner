#ifndef SEARCHFRAME_H
#define SEARCHFRAME_H

#include "searchoptions.h"
#include <QFrame>

namespace Ui {
  class SearchFrame;
}

class SearchFrame : public QFrame
{
  Q_OBJECT
public:
  SearchFrame(QWidget* pParent);

  void onOpen();

public slots:
  void onPositionChanged(size_t currentPos, size_t total);

signals:
  void findNext();
  void findPrevious();
  void searchTermChanged(const QString& sTerm);
  void searchOptionsChanged(const SearchOptions& options);

private:
  void onTextChanged(const QString& sText);
  void onUseRegularExpressionChanged(bool bOn);
  void onCaseSensitivityChanged(bool bOn);
  void setFocusOnLineEdit();
  void onExit();
  void showSearchInfo(const QString& sInfo);

private:
  Ui::SearchFrame *ui;
  SearchOptions m_options;
};

#endif // SEARCHFRAME_H
