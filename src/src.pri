CONFIG += c++11

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
    $$PWD/pypp/str.hpp \
    $$PWD/pypp/re.hpp \
    $$PWD/pypp/builtin.hpp \
    $$PWD/pypp/exceptions.hpp \
    $$PWD/pypp/slice.hpp \
    $$PWD/ElementTree.hpp \
    $$PWD/pypp/xml/etree/elementtree.hpp \
    $$PWD/InlinePatterns/BacktickPattern.h \
    $$PWD/InlinePatterns/common.h \
    $$PWD/extensions/attr_list.h \
    $$PWD/extensions/abbr.h \
    $$PWD/extensions/admonition.h \
    $$PWD/extensions/def_list.h \
    $$PWD/BlockProcessors/ListIndentProcessor.h

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
    $$PWD/InlinePatterns/BacktickPattern.cpp \
    $$PWD/InlinePatterns/common.cpp \
    $$PWD/extensions/attr_list.cpp \
    $$PWD/extensions/abbr.cpp \
    $$PWD/extensions/admonition.cpp \
    $$PWD/extensions/def_list.cpp \
    $$PWD/BlockProcessors/ListIndentProcessor.cpp
