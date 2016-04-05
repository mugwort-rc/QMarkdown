/*
 * PostProcessors.cpp
 *
 *  Created on: 2013/10/31
 *      Author: mugwort_rc
 */

#include "PostProcessors.h"

#include <QSet>
#include <QString>

#include "Markdown.h"

namespace markdown{

PostProcessor::PostProcessor(const std::weak_ptr<Markdown> &markdown_instance) :
    markdown(markdown_instance)
{}

PostProcessor::~PostProcessor(void)
{}

/*!
 * Restore raw html to the document.
 */
class RawHtmlPostprocessor : public PostProcessor
{
public:
    using PostProcessor::PostProcessor;

    /*!
     * Iterate over html stash and restore "safe" html.
     */
    QString run(const QString &text)
    {
        std::shared_ptr<Markdown> markdown = this->markdown.lock();

        QString result = text;
        for ( int i = 0; i < markdown->htmlStash.html_counter; ++i ) {
            HtmlStash::Item item = markdown->htmlStash.rawHtmlBlocks[i];
            QString html = item.first;
            bool safe = item.second;
            if ( markdown->safeMode() != Markdown::default_mode && ! safe ) {
                if ( markdown->safeMode() == Markdown::escape_mode ) {
                    html = this->escape(html);
                } else if ( markdown->safeMode() == Markdown::remove_mode ) {
                    html = QString();
                } else {
                    html = markdown->html_replacement_text();
                }
            }
            if ( this->isblocklevel(html) && ( safe || ! markdown->safeMode() ) ) {
                result = result.replace(QString("<p>%1</p>").arg(markdown->htmlStash.get_placeholder(i)), html+"\n");
            }
            result = result.replace(markdown->htmlStash.get_placeholder(i), html);
        }
        return result;
    }

    /*!
     * Basic html escaping
     */
    QString escape(const QString &html)
    {
        QString result = html;
        result = result.replace("&", "&amp;");
        result = result.replace("<", "&lt;");
        result = result.replace(">", "&gt;");
        result = result.replace("\"", "&quot;");
        return result;
    }

    bool isblocklevel(const QString &html)
    {
        QRegularExpressionMatch m = QRegularExpression("^\\<\\/?([^ >]+)").match(html);
        if ( m.hasMatch() ) {
            QChar ch = m.captured(1).at(0);
            // SPECIAL_CHARS: !, ?, @, %
            if ( SPECIAL_CHARS.contains(ch) ) {
                //! Comment, php etc...
                return true;
            }
            return util::isBlockLevel(m.captured(1));
        }
        return false;
    }

private:
    static const QSet<QChar> SPECIAL_CHARS;

};

const QSet<QChar> RawHtmlPostprocessor::SPECIAL_CHARS = {'!', '?', '@', '%'};

/*!
 * Restore valid entities
 */
class AndSubstitutePostprocessor : public PostProcessor
{
public:
    AndSubstitutePostprocessor(const std::weak_ptr<Markdown> &markdown_instance=std::weak_ptr<Markdown>()) :
        PostProcessor(markdown_instance)
    {}

    QString run(const QString &text)
    {
        return QString(text).replace(util::AMP_SUBSTITUTE, "&");
    }

};

/*!
 * Restore escaped chars
 */
class UnescapePostprocessor : public PostProcessor
{
public:
    UnescapePostprocessor(const std::weak_ptr<Markdown> &markdown_instance=std::weak_ptr<Markdown>()) :
        PostProcessor(markdown_instance),
        RE(QString("%1(\\d+)%2").arg(util::STX).arg(util::ETX))
    {}

    QString run(const QString &text)
    {
        auto unescape = [](const QRegularExpressionMatch &m) -> QString {
            int i = m.captured(1).toInt();
            return QString(QChar(i));
        };
        return pypp::re::sub(this->RE, unescape, text);
    }

private:
    QRegularExpression RE;

};

OrderedDictPostProcessors build_postprocessors(const std::shared_ptr<Markdown> &md_instance)
{
    OrderedDictPostProcessors postprocessors;
    postprocessors.append("raw_html", std::shared_ptr<PostProcessor>(new RawHtmlPostprocessor(md_instance)));
    postprocessors.append("amp_substitute", std::shared_ptr<PostProcessor>(new AndSubstitutePostprocessor()));
    postprocessors.append("unescape", std::shared_ptr<PostProcessor>(new UnescapePostprocessor()));
    return postprocessors;
}

} // end of namespace markdown