/*
 * TreeProcessors.cpp
 *
 *  Created on: 2013/10/28
 *      Author: mugwort_rc
 */

#include "TreeProcessors.h"

#include <utility>

#include <QDebug>
#include "Serializers.h"

#include "Markdown.h"

namespace markdown{

TreeProcessor::TreeProcessor(const std::weak_ptr<Markdown> &md_instance) :
    markdown(md_instance), stashed_nodes()
{}

TreeProcessor::~TreeProcessor(void)
{}

/*!
 * A Treeprocessor that traverses a tree, applying inline patterns.
 */
class InlineProcessor : public TreeProcessor
{
public:
    InlineProcessor(const std::weak_ptr<Markdown> &md) :
        TreeProcessor(md),
        placeholder_prefix(util::INLINE_PLACEHOLDER_PREFIX),
        placeholder_suffix(util::ETX),
        placeholder_length(4 + this->placeholder_prefix.size() + this->placeholder_suffix.size()),
        placeholder_re(util::INLINE_PLACEHOLDER_RE)
    {}

    ~InlineProcessor(void)
    {}

private:
    /*!
     * Generate a placeholder
     */
    std::tuple<QString, QString> makePlaceholder(const QString &/*type*/)
    {
        QString id = QString("%1").arg(this->stashed_nodes.size(), 4, 10, QChar('0'));
        QString hash = util::INLINE_PLACEHOLDER.arg(id);
        return std::make_tuple(hash, id);
    }

    /*!
     * Extract id from data string, start from index
     *
     * Keyword arguments:
     *
     * * data: string
     * * index: index, from which we start search
     *
     * Returns: placeholder id and string index, after the found placeholder.
     *
     */
    std::tuple<boost::optional<QString>, int> findPlaceholder(const QString &data, int index)
    {
        QString regexTmp = data;
        if ( index != -1 ) {
            regexTmp = data.mid(index);
        }
        QRegularExpressionMatch m = this->placeholder_re.match(regexTmp);
        if ( m.hasMatch() ) {
            return std::make_tuple(m.captured(1), index+m.capturedEnd());
        } else {
            return std::make_tuple(boost::none, index+1);
        }
    }

    /*!
     * Add node to stash
     */
    QString stashNode(const Element &node, const QString &type)
    {
        QString placeholder, id;
        std::tie(placeholder, id) = this->makePlaceholder(type);
        this->stashed_nodes[id] = std::make_tuple(boost::none, node);
        return placeholder;
    }
    QString stashNode(const QString &node, const QString &type)
    {
        QString placeholder, id;
        std::tie(placeholder, id) = this->makePlaceholder(type);
        this->stashed_nodes[id] = std::make_tuple(node, boost::none);
        return placeholder;
    }

    /*!
     * Process string with inline patterns and replace it
     * with placeholders
     *
     * Keyword arguments:
     *
     * * data: A line of Markdown text
     * * patternIndex: The index of the inlinePattern to start with
     *
     * Returns: String with placeholders.
     *
     */
    QString handleInline(const QString &data, int patternIndex = 0)
    {
        std::shared_ptr<Markdown> markdown = this->markdown.lock();

        int startIndex = 0;
        QString data_ = data;
        while ( patternIndex < markdown->inlinePatterns.size() ) {
            std::shared_ptr<Pattern> pattern = markdown->inlinePatterns.at(patternIndex);
            bool matched;
            std::tie(data_, matched, startIndex) = this->applyPattern(pattern, data_, patternIndex, startIndex);
            if ( ! matched ) {
                patternIndex += 1;
            }
        }
        return data_;
    }

    /*!
     * Process placeholders in Element.text or Element.tail
     * of Elements popped from self.stashed_nodes.
     *
     * Keywords arguments:
     *
     * * node: parent node
     * * subnode: processing node
     * * isText: bool variable, True - it's text, False - it's tail
     *
     * Returns: None
     *
     */
    void processElementText(const Element &node, const Element &subnode, bool isText=true)
    {
        QString text;
        if ( isText ) {
            if ( subnode->hasText() ) {
                text = subnode->text;
                subnode->text.clear();
            }
        } else {
            if ( subnode->hasTail() ) {
                text = subnode->tail;
                subnode->tail.clear();
            }
        }

        ElementList_t childResult = this->processPlaceholders(text, subnode, isText);

        int pos = 0;
        if ( ! isText && node != subnode ) {
            pos = node->child().indexOf(subnode) + 1;
        } else {
            pos = 0;
        }

        for ( const Element &newChild : childResult ) {
            node->insert(pos++, newChild);
        }
    }

    /*!
     * Process string with placeholders and generate ElementTree tree.
     *
     * Keyword arguments:
     *
     * * data: string with placeholders instead of ElementTree elements.
     * * parent: Element, which contains processing inline data
     *
     * Returns: list with ElementTree elements with applied inline patterns.
     *
     */
    ElementList_t processPlaceholders(const QString &data, const Element &parent, bool isText=true)
    {
        ElementList_t result;
        auto linkText = [&](const QString &text, bool atomic=false){
            if ( ! text.isEmpty() ) {
                if ( ! result.isEmpty() ) {
                    Element target = result.back();
                    if ( target->hasTail() ) {
                        target->tail = target->tail+text;
                    } else {
                        target->tail = text;
                        result.pop_back();
                        result.push_back(target);
                    }
                } else if ( ! isText ) {
                    if ( parent->hasTail() ) {
                        parent->tail = parent->tail + text;
                    } else {
                        parent->tail = text;
                    }
                } else {
                    if ( parent->hasText() ) {
                        parent->text = parent->text+text;
                        parent->atomic = atomic;
                    } else {
                        parent->text = text;
                        parent->atomic = atomic;
                    }
                }
            }
        };
        int startIndex = 0;
        QString data_ = data;
        while ( ! data_.isEmpty() ) {
            int index = data_.indexOf(this->placeholder_prefix, startIndex);
            if ( index != -1 ) {
                boost::optional<QString> id;
                int phEndIndex;
                std::tie(id, phEndIndex) = this->findPlaceholder(data_, index);
                if ( this->stashed_nodes.contains(*id) ) {
                    boost::optional<QString> str;
                    boost::optional<Element> nodeptr;
                    std::tie(str, nodeptr) = this->stashed_nodes[*id];
                    if ( index > 0 ) {
                        QString text = data_.mid(startIndex, index-startIndex);
                        linkText(text);
                    }

                    if ( ! str ) {
                        //! it's Element
                        ElementList_t nodes = {*nodeptr};
                        for ( const Element &e : (*nodeptr)->child() ) {
                            nodes.push_back(e);
                        }
                        for ( const Element &child : nodes ) {
                            if ( child->hasTail() ) {
                                if ( ! child->tail.trimmed().isEmpty() ) {
                                    Element new_node = *nodeptr;
                                    this->processElementText(new_node, child, false);
                                }
                            }
                            if ( child->hasText() ) {
                                if ( ! child->text.trimmed().isEmpty() ) {
                                    this->processElementText(child, child);
                                }
                            }
                        }
                    } else {
                        //! it's just a string
                        linkText(*str);
                        startIndex = phEndIndex;
                        continue;
                    }

                    startIndex = phEndIndex;
                    result.push_back(*nodeptr);

                } else {
                    //! wrong placeholder
                    int end = index + this->placeholder_prefix.size();
                    linkText(data_.mid(startIndex, end-startIndex));
                    startIndex = end;
                }
            } else {
                QString text = data_.mid(startIndex);
                linkText(text, isText);
                data_ = QString();
            }
        }
        return result;
    }

    /*!
     * Check if the line fits the pattern, create the necessary
     * elements, add it to stashed_nodes.
     *
     * Keyword arguments:
     *
     * * data: the text to be processed
     * * pattern: the pattern to be checked
     * * patternIndex: index of current pattern
     * * startIndex: string index, from which we start searching
     *
     * Returns: String with placeholders instead of ElementTree elements.
     *
     */
    std::tuple<QString, bool, int> applyPattern(std::shared_ptr<Pattern> pattern, const QString &data, int patternIndex, int startIndex=0)
    {
        QString regexTmp = data.mid(startIndex);
        QRegularExpressionMatch match = pattern->getCompiledRegExp().match(regexTmp);
        if ( ! match.hasMatch() ) {
            return std::make_tuple(data, false, 0);
        }
        QString leftData = data.left(startIndex);

        boost::optional<QString> result = pattern->handleMatch(match);  //!< first handleMatch (case String)
        QString placeholder;
        if ( ! result ) {
            Element node = pattern->handleMatch(ElementTree(), match);     //!< second handleMatch (case Node)
            if ( ! node ) {
                return std::make_tuple(data, true, leftData.size()+match.capturedStart(match.lastCapturedIndex()));
            }
            if ( ! node->atomic ) {
                if ( node->size() == 0 || node->hasText() ) {
                    //! We need to process current node too
                    ElementList_t nodes = {node};
                    nodes.append(node->child());
                    for ( const Element &child : nodes ) {
                        if ( child->hasText() ) {
                            QString text = child->text;
                            if ( ! child->atomic ) {
                                text = this->handleInline(text, patternIndex+1);
                            }
                            child->text = text;
                        }
                        if ( child->hasTail() ) {
                            QString tail = child->tail;
                            tail = this->handleInline(tail, patternIndex);
                            child->tail = tail;
                        }
                    }
                }
            }

            placeholder = this->stashNode(node, pattern->type());
        } else {
            placeholder = this->stashNode(*result, pattern->type());
        }

        return std::make_tuple(QString("%1%2%3%4").arg(leftData).arg(match.captured(1)).arg(placeholder).arg(match.captured(match.lastCapturedIndex())), true, 0);
    }

    /*!
     * Apply inline patterns to a parsed Markdown tree.
     *
     * Iterate over ElementTree, find elements with inline tag, apply inline
     * patterns and append newly created Elements to tree.  If you don't
     * want to process your data with inline paterns, instead of normal string,
     * use subclass AtomicString:
     *
     *     node.text = markdown.AtomicString("This will not be processed.")
     *
     * Arguments:
     *
     * * tree: ElementTree object, representing Markdown tree.
     *
     * Returns: ElementTree object with applied inline patterns.
     *
     */
    Element run(const Element &tree)
    {
        std::shared_ptr<Markdown> markdown = this->markdown.lock();

        this->stashed_nodes = StashNodes();

        try{
            ElementList_t stack = {tree};
            while ( ! stack.isEmpty() ) {
                Element currElement = stack.back();
                stack.pop_back();
                typedef QPair<Element, ElementList_t> QueueItem;
                typedef QList<QueueItem> Queue;
                Queue insertQueue;
                for ( const Element &child : currElement->child() ) {
                    if ( child->hasText() && ! child->atomic ) {
                        QString text = child->text;
                        child->text.clear();
                        ElementList_t lst = this->processPlaceholders(this->handleInline(text), child);
                        stack.append(lst);
                        insertQueue.push_back(qMakePair(child, lst));
                    }
                    if ( child->hasTail() ) {
                        QString tail = this->handleInline(child->tail);
                        Element dumby = createElement("d");
                        ElementList_t tailResult = this->processPlaceholders(tail, dumby, false);
                        if ( dumby->hasTail() ) {
                            child->tail = dumby->tail;
                        } else {
                            child->tail.clear();
                        }
                        int pos = currElement->child().indexOf(child) + 1;
                        for ( Element elem : pypp::reversed(tailResult) ) {
                            currElement->insert(pos, elem);
                        }
                    }
                    if ( child->size() > 0 ) {
                        stack.push_back(child);
                    }
                }
                for ( const QueueItem &item : insertQueue ) {
                    Element element = item.first;
                    ElementList_t lst = item.second;
                    if ( markdown->enable_attributes() ) {
                        if ( element->hasText() ) {
                            QString text = element->text;
                            text = handleAttributes(text, element);
                            element->text = text;
                        }
                    }
                    int i = 0;
                    for ( const Element &newChild : lst ) {
                        if ( markdown->enable_attributes() ) {
                            //! Processing attributes
                            if ( newChild->hasTail() ) {
                                QString text = newChild->tail;
                                text = handleAttributes(text, element);
                                newChild->tail = text;
                            }
                            if ( newChild->hasText() ) {
                                QString text = newChild->text;
                                text = handleAttributes(text, element);
                                newChild->text = text;
                            }
                        }
                        element->insert(i++, newChild);
                    }
                }
            }
            return tree;
        } catch (...) {
            qWarning() << "TreeProcessor::run() exception.";
        }
        return Element();
    }

private:
    QString placeholder_prefix;
    QString placeholder_suffix;
    unsigned int placeholder_length;
    QRegularExpression placeholder_re;

};

/*!
 * Add linebreaks to the html document.
 */
class PrettifyTreeProcessor : public TreeProcessor
{
public:
    using TreeProcessor::TreeProcessor;

private:
    /*!
     * Recursively add linebreaks to ElementTree children.
     */
    void prettifyETree(const Element &elem)
    {
        if ( util::isBlockLevel(elem->tag) && elem->tag != "code" && elem->tag != "pre" ) {
            if ( ( ! elem->hasText() || elem->text.trimmed().isEmpty() )
                 && elem->size() > 0 && util::isBlockLevel(elem->child().front()->tag) ) {
                elem->text = "\n";
            }
            for ( const Element &e : elem->child() ) {
                if ( util::isBlockLevel(e->tag) ) {
                    this->prettifyETree(e);
                }
            }
            if ( ! elem->hasTail() || elem->tail.trimmed().isEmpty() ) {
                elem->tail = "\n";
            }
        }
        if ( ! elem->hasTail() || elem->tail.trimmed().isEmpty() ) {
            elem->tail = "\n";
        }
    }

public:
    /*!
     * Add linebreaks to ElementTree root object.
     */
    Element run(const Element &root)
    {
        this->prettifyETree(root);
        //! Do <br />'s seperately as they are often in the middle of
        //! inline content and missed by _prettifyETree.
        for ( const Element &br : root->iter("br") ) {
            if ( ! br->hasTail() || br->tail.trimmed().isEmpty() ) {
                br->tail = "\n";
            } else {
                br->tail = "\n"+br->tail;
            }
        }
        //! Clean up extra empty lines at end of code blocks.
        for ( const Element &pre : root->iter("pre") ) {
            if ( pre->size() > 0 && pre->child().front()->tag == "code" ) {
                pre->child().front()->text = pypp::rstrip(pre->child().front()->text)+"\n";
                pre->child().front()->atomic = true;
            }
        }
        return Element();
    }

};

OrderedDictTreeProcessors build_treeprocessors(const std::shared_ptr<Markdown> &md_instance)
{
    OrderedDictTreeProcessors treeprocessors;
    treeprocessors.append("inline", std::shared_ptr<TreeProcessor>(new InlineProcessor(md_instance)));
    treeprocessors.append("prettify", std::shared_ptr<TreeProcessor>(new PrettifyTreeProcessor(md_instance)));
    return treeprocessors;
}

} // end of namespace markdown
