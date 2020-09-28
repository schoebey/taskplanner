#ifndef TAGEDITOR_H
#define TAGEDITOR_H

#include "id_types.h"
#include <QWidget>

class ITagProvider;
class TagEditor : public QWidget
{
  Q_OBJECT
public:
  TagEditor(ITagProvider* pTagProvider);

signals:
  void tagDeleted(tag_id tagId);
  void tagEdited(tag_id tagId, const QString& sText, const QColor& color);

private:
  void setupUi();
  void addTag();
  void deleteTag(tag_id tagId);
  void editTag(tag_id tagId, const QString& sText, const QColor& color);

  ITagProvider* m_pTagProvider = nullptr;
};

#endif // TAGEDITOR_H
