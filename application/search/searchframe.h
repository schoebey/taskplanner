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

  void onOpen();

signals:
  void findNext();
  void findPrevious();
  void searchTermChanged(const QString& sTerm);

private:
  void keyPressEvent(QKeyEvent* pEvent);
  void onTextChanged(const QString& sText);
  void setFocusOnLineEdit();
  void onExit();

private:
  Ui::SearchFrame *ui;
};

#endif // SEARCHFRAME_H
