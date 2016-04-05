/*
 * BlockProcessors.cpp
 *
 *  Created on: 2013/10/27
 *      Author: mugwort_rc
 */

#include "BlockProcessors.h"

#include <QDebug>
#include <QRegularExpression>
#include <QSet>

#include <boost/algorithm/string.hpp>

#include "Markdown.h"
#include "BlockParser.h"

namespace markdown{

BlockProcessor::BlockProcessor(const std::weak_ptr<BlockParser> &parser) :
    parser(parser), tab_length(parser.lock()->markdown.lock()->tab_length())
{}

BlockProcessor::~BlockProcessor()
{}

Element BlockProcessor::lastChild(const Element &parent)
{
    if ( parent->size() > 0 ) {
        return (*parent)[-1];
    }
    return Element();
}

std::tuple<QString, QString> BlockProcessor::detab(const QString &text)
{
    QStringList newtext, lines = text.split(QRegularExpression("\n"));
    for ( const QString &line : lines ) {
        if ( line.startsWith(QString(this->tab_length, ' ')) ) {
            newtext.push_back(line.mid(this->tab_length));
        } else if ( line.trimmed().isEmpty() ) {
            newtext.push_back(QString());
		} else {
			break;
		}
	}
    QStringList resultLines;
    int counter = 0;
    for ( QStringList::const_iterator it = lines.begin(); it != lines.end(); ++it, ++counter ) {
		if ( counter < newtext.size() ) {
			continue;
		}
		resultLines.push_back(*it);
    }
    return std::make_tuple(newtext.join("\n"), resultLines.join("\n"));
}

QString BlockProcessor::looseDetab(const QString &text, unsigned int level)
{
	const unsigned int length = this->tab_length*level;
    QStringList lines = text.split(QRegularExpression("\n"));
    for ( int i = 0; i < lines.size(); ++i ) {
        if ( lines[i].startsWith(QString(length, ' ')) ) {
            lines[i] = lines[i].mid(length);
		}
	}
    return lines.join("\n");
}

/*!
 * Process children of list items.
 *
 *  Example:
 *      * a list item
 *          process this part
 *
 *          or this part
 *
 */
class ListIndentProcessor : public BlockProcessor
{
public:
    ListIndentProcessor(const std::weak_ptr<BlockParser> &parser) :
		BlockProcessor(parser),
        INDENT_RE(QString("^(([ ]{%1})+)").arg(this->tab_length))
	{}
	~ListIndentProcessor(void)
	{}

    bool test(const Element &parent, const QString &block)
    {
        std::shared_ptr<BlockParser> parser = this->parser.lock();

        return block.startsWith(QString(this->tab_length, ' '))
                && ! parser->state.isstate("detabbed")
                && ( this->ITEM_TYPES.contains(parent->tag)
                || ( parent->size() > 0
                     && this->LIST_TYPES.contains((*parent)[-1]->tag) )
                );
	}

    void run(const Element &parent, QStringList &blocks)
    {
        std::shared_ptr<BlockParser> parser = this->parser.lock();

        QString block = blocks.front();
        blocks.pop_front();
        int level;
        Element sibling;
        std::tie(level, sibling) = this->get_level(parent, block);
        block = this->looseDetab(block, level);

        parser->state.set("detabbed");
        if ( this->ITEM_TYPES.contains(parent->tag) ) {
            //! It's possible that this parent has a 'ul' or 'ol' child list
            //! with a member.  If that is the case, then that should be the
            //! parent.  This is intended to catch the edge case of an indented
            //! list whose first member was parsed previous to this point
            //! see OListProcessor
            if ( parent->size() > 0 && this->LIST_TYPES.contains((*parent)[-1]->tag) ) {
                QStringList new_blocks = {block};
                Element new_parent = (*parent)[-1];
                parser->parseBlocks(new_parent, new_blocks);
            } else {
                QStringList new_blocks = {block};
                //! The parent is already a li. Just parse the child block.
                parser->parseBlocks(parent, new_blocks);
            }
        } else if ( this->ITEM_TYPES.contains(sibling->tag) ) {
            //! The sibling is a li. Use it as parent.
            QStringList new_blocks = {block};
            parser->parseBlocks(sibling, new_blocks);
        } else if ( sibling->size() > 0 && this->ITEM_TYPES.contains((*sibling)[-1]->tag) ) {
            //! The parent is a list (``ol`` or ``ul``) which has children.
            //! Assume the last child li is the parent of this block.
            if ( (*sibling)[-1]->hasText() ) {
                //! If the parent li has text, that text needs to be moved to a p
                //! The p must be 'inserted' at beginning of list in the event
                //! that other children already exist i.e.; a nested sublist.
                Element elem = (*sibling)[-1];
                QString text = elem->text;
                elem->text.clear();
                Element p = createElement("p");
                p->text = text;
                if ( elem->size() > 0 ) {
                    elem->insert(0, p);
                } else {
                    elem->append(p);
                }
            }
            Element new_parent = (*sibling)[-1];
            parser->parseChunk(new_parent, block);
        } else {
            this->create_item(sibling, block);
        }
        parser->state.reset();
    }

    /*!
     * Create a new li and parse the block with it as the parent.
     */
    void create_item(const Element &parent, const QString &block)
    {
        std::shared_ptr<BlockParser> parser = this->parser.lock();

        Element li = createSubElement(parent, "li");
        QStringList new_blocks = {block};
        parser->parseBlocks(li, new_blocks);
    }

    /*!
     * Get level of indent based on list level.
     */
    std::tuple<int, Element> get_level(const Element &parent, const QString &block)
    {
        std::shared_ptr<BlockParser> parser = this->parser.lock();

        //! Get indent level
        int indent_level = 0;
        int level = 0;
        QRegularExpressionMatch m = this->INDENT_RE.match(block);
        if ( m.hasMatch() ) {
            indent_level = m.captured(1).size()/this->tab_length;
        }
        if ( parser->state.isstate("list") ) {
            //! We're in a tightlist - so we already are at correct parent.
            level = 1;
        } else {
            //! We're in a looselist - so we need to find parent.
            level = 0;
        }
        //! Step through children of tree to find matching indent level.
        Element parent_ = parent;
        while ( indent_level > level ) {
            Element child = this->lastChild(parent_);
            if ( child && ( this->LIST_TYPES.contains(child->tag) || this->ITEM_TYPES.contains(child->tag) ) ) {
                if ( this->LIST_TYPES.contains(child->tag) ) {
                    level += 1;
                }
                parent_ = child;
            } else {
                //! No more child levels. If we're short of indent_level,
                //! we have a code block. So we stop here.
                break;
            }
        }
        return std::make_tuple(level, parent_);
    }

private:
    QRegularExpression INDENT_RE;

private:
    static const QSet<QString> ITEM_TYPES;
    static const QSet<QString> LIST_TYPES;

};

const QSet<QString> ListIndentProcessor::ITEM_TYPES = {"li"};
const QSet<QString> ListIndentProcessor::LIST_TYPES = {"ul", "ol"};

/*!
 * Process code blocks.
 */
class CodeBlockProcessor : public BlockProcessor
{
public:
    using BlockProcessor::BlockProcessor;

    bool test(const Element &, const QString &block)
    {
        return block.startsWith(QString(this->tab_length, ' '));
    }

    void run(const Element &parent, QStringList &blocks)
    {
        Element sibling = this->lastChild(parent);
        QString block = blocks.front();
        blocks.pop_front();
        QString theRest;
        if ( sibling && sibling->tag == "pre" && sibling->size() > 0 && (*sibling)[0]->tag == "code" ) {
            //! The previous block was a code block. As blank lines do not start
            //! new code blocks, append this block to the previous, adding back
            //! linebreaks removed from the split into a list.
            Element code = (*sibling)[0];
            std::tie(block, theRest) = this->detab(block);
            code->text = QString("%1\n%2\n").arg(code->text).arg(pypp::rstrip(block));
            code->atomic = true;
        } else {
            //! This is a new codeblock. Create the elements and insert text.
            Element pre = createSubElement(parent, "pre");
            Element code = createSubElement(pre, "code");
            std::tie(block, theRest) = this->detab(block);
            code->text = QString("%1\n").arg(pypp::rstrip(block));
            code->atomic = true;
        }
        if ( ! theRest.isEmpty() ) {
            //! This block contained unindented line(s) after the first indented
            //! line. Insert these lines as the first block of the master blocks
            //! list for future processing.
            blocks.push_front(theRest);
        }
    }

};

class BlockQuoteProcessor : public BlockProcessor
{
public:
    BlockQuoteProcessor(const std::weak_ptr<BlockParser> &parser) :
        BlockProcessor(parser),
        RE("(^|\\n)[ ]{0,3}>[ ]?(.*)")
    {}

    bool test(const Element &, const QString &block)
    {
        return this->RE.match(block).hasMatch();
    }

    void run(const Element &parent, QStringList &blocks)
    {
        std::shared_ptr<BlockParser> parser = this->parser.lock();

        QString block = blocks.front();
        blocks.pop_front();
        QRegularExpressionMatch m = this->RE.match(block);
        if ( m.hasMatch() ) {
            QString before = block.left(m.capturedStart());  //!< Lines before blockquote
            //! Pass lines before blockquote in recursively for parsing forst.
            QStringList new_blocks = {before};
            parser->parseBlocks(parent, new_blocks);
            //! Remove ``> `` from begining of each line.
            QString after = block.mid(m.capturedStart());
            QStringList lines = after.split("\n");
            QStringList new_lines;
            for ( const QString &line : lines ) {
                new_lines.push_back(this->clean(line));
            }
            block = new_lines.join("\n");
        }
        Element sibling = this->lastChild(parent);
        Element quote = Element();
        if ( sibling && sibling->tag == "blockquote" ) {
            //! Previous block was a blockquote so set that as this blocks parent
            quote = sibling;
        } else {
            //! This is a new blockquote. Create a new parent element.
            quote = createSubElement(parent, "blockquote");
        }
        //! Recursively parse block with blockquote as parent.
        //! change parser state so blockquotes embedded in lists use p tags
        parser->state.set("blockquote");
        parser->parseChunk(quote, block);
        parser->state.reset();
    }

    /*!
     * Remove ``>`` from beginning of a line.
     */
    QString clean(const QString &line)
    {
        QRegularExpressionMatch m = this->RE.match(line);
        if ( line.trimmed() == ">" ) {
            return QString();
        } else if ( m.hasMatch() ) {
            return m.captured(2);
        } else {
            return line;
        }
    }

private:
    const QRegularExpression RE;

};

/*!
 * Process ordered list blocks.
 */
class OListProcessor : public BlockProcessor
{
public:
    OListProcessor(const std::weak_ptr<BlockParser> &parser) :
        BlockProcessor(parser),
        TAG("ol"),
        RE(QString("^[ ]{0,%1}\\d+\\.[ ]+(.*)").arg(this->tab_length-1)),
        CHILD_RE(QString("^[ ]{0,%1}((\\d+\\.)|[*+-])[ ]+(.*)").arg(this->tab_length-1)),
        INDENT_RE(QString("^[ ]{%1,%2}((\\d+\\.)|[*+-])[ ]+.*").arg(this->tab_length).arg(this->tab_length*2-1)),
        STARTSWITH("1"),
        SIBLING_TAGS({"ol", "ul"})
    {}
    virtual ~OListProcessor()
    {}

    bool test(const Element &, const QString &block)
    {
        return this->RE.match(block).hasMatch();
    }

    void run(const Element &parent, QStringList &blocks)
    {
        std::shared_ptr<BlockParser> parser = this->parser.lock();

        //! Check fr multiple items in one block.
        QString block = blocks.front();
        blocks.pop_front();
        QStringList items = this->get_items(block);
        Element sibling = this->lastChild(parent);
        Element lst;

        if ( sibling && this->SIBLING_TAGS.contains(sibling->tag) ) {
            //! Previous block was a list item, so set that as parent
            lst = sibling;
            //! make sure previous item is in a p- if the item has text, then it
            //! it isn't in a p
            if ( lst->size() > 0 && (*lst)[-1]->hasText() ) {
                //! since it's possible there are other children for this sibling,
                //! we can't just SubElement the p, we need to insert it as the
                //! first item
                Element elem = (*lst)[-1];
                Element p = createElement("p");
                p->text = elem->text;
                elem->text.clear();
                if ( elem->size() > 0 ) {
                    elem->insert(0, p);
                } else {
                    elem->append(p);
                }
            }
            //! if the last item has a tail, then the tail needs to be put in a p
            //! likely only when a header is not followed by a blank line
            Element lch = this->lastChild((*lst)[-1]);
            if ( lch && lch->hasTail() ) {
                Element p = createSubElement((*lst)[-1], "p");
                p->text = pypp::lstrip(lch->tail);
                lch->tail.clear();
            }

            //! parse first block differently as it gets wrapped in a p.
            Element li = createSubElement(lst, "li");
            parser->state.set("looselist");
            QString firstitem = items.front();
            items.pop_front();
            QStringList new_blocks = {firstitem};
            parser->parseBlocks(li, new_blocks);
            parser->state.reset();
        } else if ( parent->tag == "ol" || parent->tag == "ul" ) {
            //! this catches the edge case of a multi-item indented list whose
            //! first item is in a blank parent-list item:
            //! * * subitem1
            //!     * subitem2
            //! see also ListIndentProcessor
            lst = parent;
        } else {
            //! This is a new list so create parent with appropriate tag.
            lst = createSubElement(parent, this->TAG);
            //! Check if a custom start integer is set
            if ( ! parser->markdown.lock()->lazy_ol() && this->STARTSWITH != "1" ) {
                lst->set("start", this->STARTSWITH);
            }
        }

        parser->state.set("list");
        //! Loop through items in block, recursively parsing each with the
        //! appropriate parent.
        for ( const QString &item : items ) {
            QStringList new_blocks = {item};
            if ( item.startsWith(QString(this->tab_length, ' ')) ) {
                Element new_parent = (*lst)[-1];
                //! Item is indented. Parse with last item as parent
                parser->parseBlocks(new_parent, new_blocks);
            } else {
                //! New item. Create li and parse with it as parent
                Element li = createSubElement(lst, "li");
                parser->parseBlocks(li, new_blocks);
            }
        }
        parser->state.reset();
    }

    /*!
     * Break a block into list items.
     */
    QStringList get_items(const QString &block)
    {
        QStringList items;
        QStringList lines = block.split("\n");
        for ( const QString &line : lines ) {
            QRegularExpressionMatch m = this->CHILD_RE.match(line);
            if ( m.hasMatch() ) {
                //! This is a new list item
                //! Check first item for the start index
                if ( items.empty() && this->TAG == "ol" ) {
                    //! Detect the integer value of first list item
                    QRegularExpression INTEGER_RE("(\\d+)");
                    QRegularExpressionMatch im = INTEGER_RE.match(m.captured(1));
                    this->STARTSWITH = im.captured();
                }
                //! Append to the list
                items.push_back(m.captured(3));
            } else if ( this->INDENT_RE.match(line).hasMatch() ) {
                //! This is an indented (possibly nested) item.
                if ( ! items.empty() && items.back().startsWith(QString(this->tab_length, ' ')) ) {
                    //! Previous item was indented. Append to that item.
                    items.back() = QString("%1\n%2").arg(items.back()).arg(line);
                } else {
                    items.push_back(line);
                }
            } else {
                //! This is another line of previous item. Append to that item.
                items.back() = QString("%1\n%2").arg(items.back()).arg(line);
            }
        }
        return items;
    }

protected:
    QString TAG;
    //! Detect an item (``1. item``). ``group(1)`` contains contents of item.
    QRegularExpression RE;

private:
    //! Detect items on secondary lines. they can be of either list type.
    const QRegularExpression CHILD_RE;
    //! Detect indented (nested) items of either type
    const QRegularExpression INDENT_RE;
    //! The integer (python string) with which the lists starts (default=1)
    //! Eg: If list is intialized as)
    //!   3. Item
    //! The ol tag will get starts="3" attribute
    QString STARTSWITH;
    //! List of allowed sibling tags.
    const QSet<QString> SIBLING_TAGS;

};

class UListProcessor : public OListProcessor
{
public:
    UListProcessor(const std::weak_ptr<BlockParser> &parser) :
        OListProcessor(parser)
    {
        OListProcessor::TAG = "ul";
        OListProcessor::RE = QRegularExpression(QString("^[ ]{0,%1}[*+-][ ]+(.*)").arg(this->tab_length-1));
    }

};

/*!
 * Process Hash Headers.
 */
class HashHeaderProcessor : public BlockProcessor
{
public:
    HashHeaderProcessor(const std::weak_ptr<BlockParser> &parser) :
        BlockProcessor(parser),
        RE("(^|\\n)(?<level>#{1,6})(?<header>.*?)#*(\\n|$)")
    {}

    bool test(const Element &, const QString &block)
    {
        return this->RE.match(block).hasMatch();
    }

    void run(const Element &parent, QStringList &blocks)
    {
        std::shared_ptr<BlockParser> parser = this->parser.lock();

        QString block = blocks.front();
        blocks.pop_front();
        QRegularExpressionMatch m = this->RE.match(block);
        if ( m.hasMatch() ) {
            QString before = block.left(m.capturedStart());  //!< All lines before header
            QString after  = block.mid(m.capturedEnd()); //!< All lines after header
            if ( ! before.isEmpty() ) {
                //! As the header was not the first line of the block and the
                //! lines before the header must be parsed first,
                //! recursively parse this lines as a block.
                QStringList new_blocks = {before};
                parser->parseBlocks(parent, new_blocks);
            }
            //! Create header using named groups from RE
            Element h = createSubElement(parent, QString("h%1").arg(m.captured("level").size()));
            h->text = m.captured("header").trimmed();
            if ( ! after.isEmpty() ) {
                //! Insert remaining lines as first block for future parsing.
                blocks.push_front(after);
            }
        } else {
            //! This should never happen, but just in case...
            qWarning() << "We've got a problem header: " << block;
        }
    }

private:
    //! Detect a header at start of any line in block
    QRegularExpression RE;

};

/*!
 * Process Setext-style Headers.
 */
class SetextHeaderProcessor : public BlockProcessor
{
public:
    SetextHeaderProcessor(const std::weak_ptr<BlockParser> &parser) :
        BlockProcessor(parser),
        RE("^.*?\\n[=-]+[ ]*(\\n|$)", QRegularExpression::MultilineOption)
    {}

    bool test(const Element &, const QString &block)
    {
        return this->RE.match(block).hasMatch();
    }

    void run(const Element &parent, QStringList &blocks)
    {
        QString block = blocks.front();
        blocks.pop_front();
        QStringList lines = block.split("\n");
        //! Determine level. ``=`` is 1 and ``-`` is 2.
        int level = 0;
        if ( lines.at(1).startsWith("=") ) {
            level = 1;
        } else {
            level = 2;
        }
        Element h = createSubElement(parent, QString("h%1").arg(level));
        h->text = lines.at(0).trimmed();
        if ( lines.size() > 2 ) {
            //! Block contains additional lines. Add to  master blocks for later.
            QStringList buff;
            int counter = 0;
            for ( const QString &line : lines ) {
                if ( counter++ < 2 ) {
                    continue;
                }
                buff.push_back(line);
            }
            blocks.push_front(buff.join("\n"));
        }
    }

private:
    //! Detect Setext-style header. Must be first 2 lines of block.
    const QRegularExpression RE;

};

/*!
 * Process Horizontal Rules.
 */
class HRProcessor : public BlockProcessor
{
public:
    HRProcessor(const std::weak_ptr<BlockParser> &parser) :
        BlockProcessor(parser),
        SEARCH_RE("^[ ]{0,3}((-+[ ]{0,2}){3,}|(_+[ ]{0,2}){3,}|(\\*+[ ]{0,2}){3,})[ ]*", QRegularExpression::MultilineOption),
        match()
    {}

    bool test(const Element &, const QString &block)
    {
        //! No atomic grouping in python so we simulate it here for performance.
        //! The regex only matches what would be in the atomic group - the HR.
        //! Then check if we are at end of block or if next char is a newline.
        QRegularExpressionMatch m = this->SEARCH_RE.match(block);
        if ( m.hasMatch()
             && ( m.capturedEnd() == block.size()
                  || block.at(m.capturedStart()+m.capturedLength()) == '\n' ) ) {
            //! Save match object on class instance so we can use it later.
            this->match = m;
            return true;
        }
        return false;
    }

    void run(const Element &parent, QStringList &blocks)
    {
        std::shared_ptr<BlockParser> parser = this->parser.lock();

        QString block = blocks.front();
        blocks.pop_front();
        //! Check for lines in block before hr.
        QString prelines = pypp::rstrip(block.left(this->match.capturedStart()), [](const QChar &ch) -> bool { return ch == '\n'; });
        if ( ! prelines.isEmpty() ) {
            //! Recursively parse lines before hr so they get parsed first.
            QStringList new_blocks = {prelines};
            parser->parseBlocks(parent, new_blocks);
        }
        //! create hr
        Element hr = createSubElement(parent, "hr");
        //! check for lines in block after hr.
        int begin = this->match.capturedStart()+this->match.capturedLength();
        QString postlines = pypp::lstrip(block.mid(begin), [](const QChar &ch) -> bool { return ch == '\n'; });
        if ( ! postlines.isEmpty() ) {
            //! Add lines after hr to master blocks for later parsing.
            blocks.push_front(postlines);
        }
    }

private:
    //! Detect hr on any line of a block.
    QRegularExpression SEARCH_RE;
    QRegularExpressionMatch match;

};

/*!
 * Process blocks that are empty or start with an empty line.
 */
class EmptyBlockProcessor : public BlockProcessor
{
public:
    using BlockProcessor::BlockProcessor;

    bool test(const Element &, const QString &block)
    {
        return block.isEmpty() || block.startsWith('\n');
    }

    void run(const Element &parent, QStringList &blocks)
    {
        QString block = blocks.front();
        blocks.pop_front();
        QString filler = "\n\n";
        if ( ! block.isEmpty() ) {
            //! Starts with empty line
            //! Only replace a single line.
            filler = "\n";
            //! Save the rest for later.
            QString theRest = block.mid(1);
            if ( ! theRest.isEmpty() ) {
                //! Add remaining lines to master blocks for later.
                blocks.push_front(theRest);
            }
        }
        Element sibling = this->lastChild(parent);
        if ( sibling && sibling->tag == "pre" && sibling->size() > 0 && (*sibling)[0]->tag == "code" ) {
            //! Last block is a codeblock. Append to preserve whitespace.
            QString codeText = (*sibling)[0]->text;
            (*sibling)[0]->text = QString("%1%2").arg(codeText).arg(filler);
            (*sibling)[0]->atomic = true;
        }
    }

};

/*!
 * Process Paragraph blocks.
 */
class ParagraphProcessor : public BlockProcessor
{
public:
    using BlockProcessor::BlockProcessor;

    bool test(const Element &, const QString &)
    {
        return true;
    }

    void run(const Element &parent, QStringList &blocks)
    {
        std::shared_ptr<BlockParser> parser = this->parser.lock();

        QString block = blocks.front();
        blocks.pop_front();
        if ( ! block.trimmed().isEmpty() ) {
            //! Not a blank block. Add to parent, otherwise throw it away.
            if ( parser->state.isstate("list") ) {
                //! The parent is a tight-list.
                //!
                //! Check for any children. This will likely only happen in a
                //! tight-list when a header isn't followed by a blank line.
                //! For example:
                //!
                //!     * # Header
                //!     Line 2 of list item - not part of header.
                Element sibling = this->lastChild(parent);
                if ( sibling ) {
                    //! Insetrt after sibling.
                    if ( sibling->hasTail() ) {
                        QString tailText = sibling->tail;
                        sibling->tail = QString("%1\n%2").arg(tailText).arg(block);
                    } else {
                        sibling->tail = QString("\n%1").arg(block);
                    }
                } else {
                    //! Append to parent.text
                    if ( parent->hasText() ) {
                        QString parentText = parent->text;
                        parent->text = QString("%1\n%2").arg(parentText).arg(block);
                    } else {
                        parent->text = pypp::lstrip(block);
                    }
                }
            } else {
                //! Create a regular paragraph
                Element p = createSubElement(parent, "p");
                p->text = pypp::lstrip(block);
            }
        }
    }

};

std::shared_ptr<BlockParser> build_block_parser(const std::shared_ptr<Markdown> &md_instance)
{
    std::shared_ptr<BlockParser> parser(new BlockParser(md_instance));
    parser->blockprocessors.append("empty", std::shared_ptr<BlockProcessor>(new EmptyBlockProcessor(parser)));
    parser->blockprocessors.append("indent", std::shared_ptr<BlockProcessor>(new ListIndentProcessor(parser)));
    parser->blockprocessors.append("code", std::shared_ptr<BlockProcessor>(new CodeBlockProcessor(parser)));
    parser->blockprocessors.append("hashheader", std::shared_ptr<BlockProcessor>(new HashHeaderProcessor(parser)));
    parser->blockprocessors.append("setextheader", std::shared_ptr<BlockProcessor>(new SetextHeaderProcessor(parser)));
    parser->blockprocessors.append("hr", std::shared_ptr<BlockProcessor>(new HRProcessor(parser)));
    parser->blockprocessors.append("olist", std::shared_ptr<BlockProcessor>(new OListProcessor(parser)));
    parser->blockprocessors.append("ulist", std::shared_ptr<BlockProcessor>(new UListProcessor(parser)));
    parser->blockprocessors.append("quote", std::shared_ptr<BlockProcessor>(new BlockQuoteProcessor(parser)));
    parser->blockprocessors.append("paragraph", std::shared_ptr<BlockProcessor>(new ParagraphProcessor(parser)));
	return parser;
}

} // end of namespace markdown
