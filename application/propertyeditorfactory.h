#ifndef PROPERTYEDITORFACTORY_H
#define PROPERTYEDITORFACTORY_H

#include <QString>
#include <QObject>

#include <map>
#include <memory>

/*
usage:
PropertyEditorFactory::registerEditor<EditableLabel, TaskWidget>("due date");
QWidget* pEditor = PropertyEditorFactory::createAndConnect<TaskWidget>("due date", this);
*/
class QWidget;
namespace detail {
  // because of memory issues (accessing a static object from different translation units),
  // the map needs to be in the cpp file, thus the need
  // for it to store non-template classes.
  // This is why all the creators need a common base class.
  class creator_base_base
  {
  public:
    creator_base_base(const QString& s) : m_sPropertyName(s) {}
    virtual ~creator_base_base() {}
    QString m_sPropertyName;
  };

  template<typename T>
  class creator_base : public creator_base_base
  {
  public:
    creator_base(const QString& s) : creator_base_base(s) {}
    virtual QWidget* create(QWidget* pParent) const = 0;
    virtual QWidget* createAndConnect(QWidget*, T*) const = 0;
  };


  template<typename EditorClass, typename T> class creator : public creator_base<T>
  {
  public:
    creator(const QString& s, std::function<void(EditorClass*)> initializer)
      : creator_base<T>(s),
        m_initializer(initializer) {}
    QWidget* create(QWidget* pParent) const override
    {
      EditorClass* pEditor = new EditorClass(pParent);
      if (m_initializer) { m_initializer(pEditor); }
      return pEditor;
    }
    QWidget* createAndConnect(QWidget* pParent, T* pReceiver) const override
    {
      EditorClass* pEditor = new EditorClass(pParent);
      QString sPropertyName = this->m_sPropertyName;
      QObject::connect(pEditor, &EditorClass::valueChanged,
              [pReceiver, sPropertyName](const QString& sVal)
      {
        pReceiver->onPropertyValueChanged(sPropertyName, sVal);
      });
      QObject::connect(pReceiver, &T::propertyValueChanged,
                       [pEditor, sPropertyName](const QString& sName,
                                                const QString& sValue)
      {
        if (sName == sPropertyName)
        {
          pEditor->setValue(sValue);
        }
      });

      if (m_initializer) { m_initializer(pEditor); }
      return pEditor;
    }
  private:
    std::function<void(EditorClass*)> m_initializer;
  };
}

class PropertyEditorFactory
{
private:
  static void registerCreator(const QString& sPropertyName,
                              detail::creator_base_base* pCreator);

  static std::shared_ptr<detail::creator_base_base> creator(const QString& sPropertyName);
public:
  template<typename EditorClass, typename T>
  // TODO: enforce the necessary signals/slots via enable_if
  static void registerEditor(const QString& sPropertyName, std::function<void(EditorClass*)> initializer = nullptr)
  {
    registerCreator(sPropertyName, new detail::creator<EditorClass, T>(sPropertyName, initializer));
  }

  template<typename T>
  static QWidget* createAndConnect(const QString& sPropertyName, T* pReceiver)
  {
    auto spCreator = creator(sPropertyName);
    auto spCastCreator = std::dynamic_pointer_cast<detail::creator_base<T>>(spCreator);
    if (nullptr != spCastCreator)
    {
      return spCastCreator->createAndConnect(pReceiver, pReceiver);
    }

    return nullptr;
  }

private:
};


#endif // PROPERTYEDITORFACTORY_H
