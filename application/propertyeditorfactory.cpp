#include "propertyeditorfactory.h"

namespace  {
  std::map<QString, std::shared_ptr<detail::creator_base_base>> creators;
}

void PropertyEditorFactory::registerCreator(const QString& sPropertyName,
                                            detail::creator_base_base* pCreator)
{
  creators[sPropertyName] = std::shared_ptr<detail::creator_base_base>(pCreator);
}

std::shared_ptr<detail::creator_base_base>
PropertyEditorFactory::creator(const QString& sPropertyName)
{
  auto it = creators.find(sPropertyName);
  if (it != creators.end())
  {
    return it->second;
  }
  return nullptr;
}
