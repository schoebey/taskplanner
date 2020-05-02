#ifndef COMBINEDUNDOCOMMAND_H
#define COMBINEDUNDOCOMMAND_H

#include <QUndoCommand>

class CombinedUndoCommand : public QUndoCommand
{
public:
  CombinedUndoCommand(QUndoCommand* pCommand1,
                      QUndoCommand* pCommand2);
  ~CombinedUndoCommand();


  void undo() override;

  void redo() override;


private:
  QUndoCommand* m_pCommand1 = nullptr;
  QUndoCommand* m_pCommand2 = nullptr;
};

#endif // COMBINEDUNDOCOMMAND_H
