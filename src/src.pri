CONFIG += c++11
QT += xml

HEADERS += \
    $$PWD/BlockParser.h \
    $$PWD/BlockProcessors.h \
    $$PWD/InlinePatterns.h \
    $$PWD/Markdown.h \
    $$PWD/PostProcessors.h \
    $$PWD/PreProcessors.h \
    $$PWD/Serializers.h \
    $$PWD/TreeProcessors.h \
    $$PWD/htmlentitydefs.hpp \
    $$PWD/util.h \
    $$PWD/extensions/Extension.h \
    $$PWD/extensions/tables.h \
    $$PWD/pypp.hpp \
    $$PWD/odict.hpp \
    $$PWD/Processor.hpp \
    $$PWD/xerces.hpp \
    $$PWD/pypp/xml/etree/elementtree.h \
    $$PWD/pypp/str.hpp \
    $$PWD/pypp/re.hpp \
    $$PWD/pypp/builtin.hpp \
    $$PWD/pypp/exceptions.hpp \
    $$PWD/pypp/slice.hpp \
    $$PWD/ElementTree.hpp

SOURCES += \
    $$PWD/BlockParser.cpp \
    $$PWD/BlockProcessors.cpp \
    $$PWD/InlinePatterns.cpp \
    $$PWD/Markdown.cpp \
    $$PWD/PostProcessors.cpp \
    $$PWD/PreProcessors.cpp \
    $$PWD/Serializers.cpp \
    $$PWD/TreeProcessors.cpp \
    $$PWD/util.cpp \
    $$PWD/extensions/Extension.cpp \
    $$PWD/extensions/tables.cpp \
    $$PWD/pypp/xml/etree/elementtree.cpp
