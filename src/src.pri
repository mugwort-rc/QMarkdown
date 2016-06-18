CONFIG += c++11

HEADERS += \
    $$PWD/../include/QMarkdown/BlockParser.h \
    $$PWD/../include/QMarkdown/BlockProcessors.h \
    $$PWD/../include/QMarkdown/InlinePatterns.h \
    $$PWD/../include/QMarkdown/Markdown.h \
    $$PWD/../include/QMarkdown/PostProcessors.h \
    $$PWD/../include/QMarkdown/PreProcessors.h \
    $$PWD/../include/QMarkdown/Serializers.h \
    $$PWD/../include/QMarkdown/TreeProcessors.h \
    $$PWD/../include/QMarkdown/htmlentitydefs.hpp \
    $$PWD/../include/QMarkdown/util.h \
    $$PWD/../include/QMarkdown/extensions/Extension.h \
    $$PWD/../include/QMarkdown/extensions/tables.h \
    $$PWD/../include/QMarkdown/pypp.hpp \
    $$PWD/../include/QMarkdown/odict.hpp \
    $$PWD/../include/QMarkdown/Processor.hpp \
    $$PWD/../include/QMarkdown/pypp/str.hpp \
    $$PWD/../include/QMarkdown/pypp/re.hpp \
    $$PWD/../include/QMarkdown/pypp/builtin.hpp \
    $$PWD/../include/QMarkdown/pypp/exceptions.hpp \
    $$PWD/../include/QMarkdown/pypp/slice.hpp \
    $$PWD/../include/QMarkdown/ElementTree.hpp \
    $$PWD/../include/QMarkdown/pypp/xml/etree/elementtree.hpp \
    $$PWD/../include/QMarkdown/InlinePatterns/BacktickPattern.h \
    $$PWD/../include/QMarkdown/InlinePatterns/common.h \
    $$PWD/../include/QMarkdown/extensions/attr_list.h \
    $$PWD/../include/QMarkdown/extensions/abbr.h \
    $$PWD/../include/QMarkdown/extensions/admonition.h \
    $$PWD/../include/QMarkdown/extensions/def_list.h \
    $$PWD/../include/QMarkdown/BlockProcessors/ListIndentProcessor.h

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

INCLUDEPATH += $$PWD/../include/QMarkdown
