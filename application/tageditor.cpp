#include "tageditor.h"
#include "tagproviderinterface.h"
#include "tagwidget.h"

#include <QGridLayout>
#include <QPushButton>

TagEditor::TagEditor(ITagProvider* pTagProvider)
  : m_pTagProvider(pTagProvider)
{
  setupUi();
}

void TagEditor::setupUi()
{
  QGridLayout* pLayout = dynamic_cast<QGridLayout*>(layout());
  if (nullptr == pLayout) {
    pLayout = new QGridLayout(this);
    setLayout(pLayout);
  } else {
    QLayoutItem* pItem = nullptr;
    while (nullptr != (pItem = pLayout->takeAt(0))) {
      delete pItem->widget();
      delete pItem;
    }
  }

  if (nullptr != m_pTagProvider)
  {
    int iRow = 0;
    for (const STagData& el : m_pTagProvider->availableTags())
    {
      TagWidget* pTagWidget = new TagWidget(el.id, el.sName, this);
      pTagWidget->setColor(el.color);
      pTagWidget->setEditable(true);
      auto onTagWidgetEdited = [&]() {
        TagWidget* pTagWidget = qobject_cast<TagWidget*>(sender());
        if (nullptr != pTagWidget) {
          editTag(pTagWidget->id(), pTagWidget->text(), pTagWidget->color());
        }
      };
      connect(pTagWidget, &TagWidget::textChanged, this, onTagWidgetEdited);
      connect(pTagWidget, &TagWidget::colorChanged, this, onTagWidgetEdited);
      pLayout->addWidget(pTagWidget, iRow++, 0);
    }

    QPushButton* pNewTag = new QPushButton("add new tag", this);
    pLayout->addWidget(pNewTag, iRow, 0);
    connect(pNewTag, &QPushButton::clicked, this, &TagEditor::addTag);
  }
}

void TagEditor::addTag()
{
  if (nullptr != m_pTagProvider) {
    tag_id tagId = m_pTagProvider->addTag();
    if (tagId != tag_id{}) {
      setupUi();
    }
  }
}

void TagEditor::deleteTag(tag_id tagId)
{
  // TODO: warning dialog
  if (nullptr != m_pTagProvider) {
    m_pTagProvider->deleteTag(tagId);
  }
}

void TagEditor::editTag(tag_id tagId, const QString& sText, const QColor& color)
{
  if (nullptr != m_pTagProvider) {
    m_pTagProvider->modifyTag(tagId, sText, color);
    emit tagEdited(tagId, sText, color);
  }
}
