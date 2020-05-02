#include "combinedundocommand.h"

CombinedUndoCommand::CombinedUndoCommand(QUndoCommand* pCommand1,
                                         QUndoCommand* pCommand2)
  : m_pCommand1(pCommand1),
    m_pCommand2(pCommand2)
{

}

CombinedUndoCommand::~CombinedUndoCommand()
{
  delete m_pCommand1;
  delete m_pCommand2;
}

void CombinedUndoCommand::undo()
{
  if (nullptr != m_pCommand1)  { m_pCommand1->undo(); }
  if (nullptr != m_pCommand2)  { m_pCommand2->undo(); }
}

void CombinedUndoCommand::redo()
{
  if (nullptr != m_pCommand1)  { m_pCommand1->redo(); }
  if (nullptr != m_pCommand2)  { m_pCommand2->redo(); }
}
