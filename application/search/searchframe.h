#ifndef SEARCHFRAME_H
#define SEARCHFRAME_H

#include <QFrame>

namespace Ui {
  class SearchFrame;
}

class SearchFrame : public QFrame
{
  Q_OBJECT
public:
  SearchFrame(QWidget* pParent);


signals:
  void findNext();
  void findPrevious();
  void searchTermChanged(const QString& sTerm);

private:
  void onTextChanged(const QString& sText);
  void onExit();

private:
  Ui::SearchFrame *ui;
};

#endif // SEARCHFRAME_H
