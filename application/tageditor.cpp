#include "tageditor.h"
#include "tagproviderinterface.h"
#include "tagwidget.h"

#include <QGridLayout>

TagEditor::TagEditor(ITagProvider* pTagProvider)
  : m_pTagProvider(pTagProvider)
{
  setupUi();
}

void TagEditor::setupUi()
{
  QGridLayout* pLayout = new QGridLayout(this);
  setLayout(pLayout);

  if (nullptr != m_pTagProvider)
  {
    int iRow = 0;
    for (const STagData& el : m_pTagProvider->availableTags())
    {
      TagWidget* pTagWidget = new TagWidget(el.id, el.sName, this);
      pTagWidget->setColor(el.color);
      pLayout->addWidget(pTagWidget, iRow++, 0);
    }
  }
}
