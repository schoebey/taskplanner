include(text/text.pri)
include(markdown/markdown.pri)

INCLUDEPATH += $$PWD/../ \

SOURCES +=

HEADERS += \
    $$PWD/serializerinterface.h \
    $$PWD/serializerfactory.h \
    $$PWD/serializationenums.h \
    $$PWD/serializerregister.h
