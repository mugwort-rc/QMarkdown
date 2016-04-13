/*
 * InlinePatterns.cpp
 *
 *  Created on: 2013/10/28
 *      Author: mugwort_rc
 */

#include "InlinePatterns.h"

#include <QUrl>
#if QT_VERSION > QT_VERSION_CHECK(5,0,0)
#include <QUrlQuery>
#endif
#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
#include <QTextDocument>
#endif
#include "htmlentitydefs.hpp"

#include <utility>

#include <boost/algorithm/string.hpp>

#include <QPair>
#include <QSet>

#include "Markdown.h"
#include "pypp.hpp"

namespace markdown{

const QString NOBRACKET("[^\\]\\[]*");

const QString BRK = ( "\\[("
        + (NOBRACKET + "(\\[") + (NOBRACKET + "(\\[") + (NOBRACKET + "(\\[") + (NOBRACKET + "(\\[") + (NOBRACKET + "(\\[") + (NOBRACKET + "(\\[")
        + (NOBRACKET + "\\])*") + (NOBRACKET + "\\])*") + (NOBRACKET + "\\])*") + (NOBRACKET + "\\])*") + (NOBRACKET + "\\])*") + (NOBRACKET + "\\])*")
        + NOBRACKET + ")\\]" );
const QString NOIMG = "(?<!\\!)";

//! `e=f()` or ``e=f("`")``
const QString BACKTICK_RE = "(?<!\\\\)(`+)(.+?)(?<!`)\\2(?!`)";

//! \<
const QString ESCAPE_RE = "\\\\(.)";

//! *emphasis*
const QString EMPHASIS_RE = "(\\*)([^\\*]+)\\2";

//! **strong**
const QString STRONG_RE = "(\\*{2}|_{2})(.+?)\\2";

//! ***strongem*** or ***em*strong**
const QString EM_STRONG_RE = "(\\*|_)\\2{2}(.+?)\\2(.*?)\\2{2}";

//! ***strong**em*
const QString STRONG_EM_RE = "(\\*|_)\\2{2}(.+?)\\2{2}(.*?)\\2";

//! _smart_emphasis_
const QString SMART_EMPHASIS_RE = "(?<!\\w)(_)(?!_)(.+?)(?<!_)\\2(?!\\w)";

//! _emphasis_
const QString EMPHASIS_2_RE = "(_)(.+?)\\2";

//! [text](url) or [text](<url>) or [text](url "title")
const QString LINK_RE = NOIMG + BRK + "\\(\\s*(<.*?>|((?:(?:\\(.*?\\))|[^\\(\\)]))*?)\\s*((['\"])(.*?)\\12\\s*)?\\)";

//! ![alttxt](http://x.com/) or ![alttxt](<http://x.com/>)
const QString IMAGE_LINK_RE = "\\!" + BRK + "\\s*\\((<.*?>|([^\")]+\"[^\"]*\"|[^\\)]*))\\)";

//! [Google][3]
const QString REFERENCE_RE = NOIMG + BRK + "\\s?\\[([^\\]]*)\\]";

//! [Google]
const QString SHORT_REF_RE = NOIMG + "\\[([^\\]]+)\\]";

//! ![alt text][2]
const QString IMAGE_REFERENCE_RE = "\\!" + BRK + "\\s?\\[([^\\]]*)\\]";

//! stand-alone * or _
const QString NOT_STRONG_RE = "((^| )(\\*|_)( |$))";

//!  <http://www.123.com>
const QString AUTOLINK_RE = "<((?:[Ff]|[Hh][Tt])[Tt][Pp][Ss]?://[^>]*)>";

//! <me@example.com>
const QString AUTOMAIL_RE = "<([^> \\!]*@[^> ]*)>";

//! <...>
const QString HTML_RE = "(<([a-zA-Z/][^>]*?|\\!--.*?--)>)";

//! &amp;
const QString ENTITY_RE = "(&[\\#a-zA-Z0-9]*;)";

//! two spaces at end of line
const QString LINE_BREAK_RE = "  \\n";

//! Remove quotes from around a string.
QString dequote(const QString &string)
{
    if ( ( string.startsWith('"') && string.endsWith('"') ) || ( string.startsWith('\'') && string.endsWith('\'') ) ) {
        return string.mid(1, string.size()-2);
    } else {
        return string;
    }
}

QString itertext(const Element &elem)
{
    QString tag = elem->tag;
    if ( tag.isEmpty() ) {
        return QString();
    }
    QString result = elem->text;
    for ( int i = 0; i < elem->size(); ++i ) {
        result += itertext((*elem)[i]);
        result += (*elem)[i]->tail;
    }
    return result;
}

const QRegularExpression ATTR_RE("\\{@([^\\}]*)=([^\\}]*)\\}");

QString handleAttributes(const QString &text, const Element &parent)
{
    return pypp::re::sub(ATTR_RE, [&](const QRegularExpressionMatch &m) -> QString {
        parent->set(m.captured(1), m.captured(2).replace("\n",  " "));
        return pypp::str();
    }, text);
}

//! Set values of an element based on attribute definitions ({@id=123}).

Pattern::Pattern(const QString &pattern, const std::weak_ptr<Markdown> &markdown_instance) :
    pattern(pattern), compiled_re(QString("^(.*?)%1(.*)$").arg(pattern), QRegularExpression::DotMatchesEverythingOption | QRegularExpression::UseUnicodePropertiesOption),
    //! Api for Markdown to pass safe_mode into instance
    safe_mode(false), markdown(markdown_instance)
{}

QString Pattern::unescape(const QString &text)
{
    std::shared_ptr<Markdown> markdown = this->markdown.lock();

    TreeProcessor::StashNodes stash;
    if ( markdown->treeprocessors.exists("inline") ) {
        stash = markdown->treeprocessors["inline"]->stashed_nodes;
    } else {
        return text;
    }
    auto get_stash = [&](const QRegularExpressionMatch &m) -> QString {
        QString id = m.captured(1);
        if ( stash.contains(id) ) {
            boost::optional<QString> str;
            boost::optional<Element> node;
            std::tie(str, node) = stash[id];
            if ( str ) {
                return *str;
            }
            return itertext(*node);
        }
        return QString();
    };
    return pypp::re::sub(util::INLINE_PLACEHOLDER_RE, get_stash, text);
}

/*!
 * Return a simple text of group(2) of a Pattern.
 */
class SimpleTextPattern : public Pattern
{
public:
    SimpleTextPattern(const QString &pattern, const std::weak_ptr<Markdown> &md=std::weak_ptr<Markdown>()) :
        Pattern(pattern, md)
    {}

    boost::optional<QString> handleMatch(const QRegularExpressionMatch &m)
    {
        QString text = m.captured(2);
        if ( text == util::INLINE_PLACEHOLDER_PREFIX ) {
            return boost::none;
        }
        return text;
    }

    QString type(void) const
    { return "SimpleTextPattern"; }

};

/*!
 * Return an escaped character.
 */
class EscapePattern : public Pattern
{
public:
    EscapePattern(const QString &pattern, const std::weak_ptr<Markdown> &md) :
        Pattern(pattern, md)
    {}

    boost::optional<QString> handleMatch(const QRegularExpressionMatch &m)
    {
        QString text = m.captured(2);
        if ( text.size() > 1 ) {
            return QString("\\%1").arg(text);
        }
        QChar ch = text.at(0);
        if ( this->markdown.lock()->ESCAPED_CHARS.contains(ch) ) {
            return QString("%1%2%3").arg(util::STX).arg(ch.unicode()).arg(util::ETX);
        } else {
            return boost::none;
        }
    }

    QString type(void) const
    { return "EscapePattern"; }

};

/*!
 * Return element of type `tag` with a text attribute of group(3)
 * of a Pattern.
 */
class SimpleTagPattern : public Pattern
{
public:
    SimpleTagPattern(const QString &pattern, const QString &tag, const std::weak_ptr<Markdown> &md=std::weak_ptr<Markdown>()) :
        Pattern(pattern, md),
        tag(tag)
    {}
    virtual ~SimpleTagPattern()
    {}

    virtual Element handleMatch(const ElementTree &, const QRegularExpressionMatch &m)
    {
        Element el = createElement(this->tag);
        el->text = m.captured(3);
        return el;
    }

    virtual QString type(void) const
    { return "SimpleTagPattern"; }

protected:
    QString tag;

};

/*!
 * Return an element of type `tag` with no children.
 */
class SubstituteTagPattern : public SimpleTagPattern
{
public:
    SubstituteTagPattern(const QString &pattern, const QString &tag, const std::weak_ptr<Markdown> &md=std::weak_ptr<Markdown>()) :
        SimpleTagPattern(pattern, tag, md)
    {}

    Element handleMatch(const ElementTree &, const QRegularExpressionMatch &)
    {
        return createElement(this->tag);
    }

    QString type(void) const
    { return "SubstituteTagPattern"; }

};

/*!
 * Return a `<code>` element containing the matching text.
 */
class BacktickPattern : public Pattern
{
public:
    BacktickPattern(const QString &pattern, const std::weak_ptr<Markdown> &md=std::weak_ptr<Markdown>()) :
        Pattern(pattern, md),
        tag("code")
    {}

    virtual Element handleMatch(const ElementTree &, const QRegularExpressionMatch &m)
    {
        Element el = createElement(this->tag);
        el->text = m.captured(3).trimmed();
        el->atomic = true;
        return el;
    }

    virtual QString type(void) const
    { return "BacktickPattern"; }

private:
    QString tag;

};

/*!
 * Return a Element element nested in tag2 nested in tag1.
 *
 * Useful for strong emphasis etc.
 *
 */
class DoubleTagPattern : public SimpleTagPattern
{
public:
    DoubleTagPattern(const QString &pattern, const QString &tag, const std::weak_ptr<Markdown> &md=std::weak_ptr<Markdown>()) :
        SimpleTagPattern(pattern, tag, md)
    {}

    Element handleMatch(const ElementTree &, const QRegularExpressionMatch &m)
    {
        QStringList tags = this->tag.split(",");
        Element el1 = createElement(tags.at(0));
        Element el2 = createSubElement(el1, tags.at(1));
        el2->text = m.captured(3);
        if ( ! m.captured(4).isEmpty() ) {
            el2->tail = m.captured(4);
        }
        return el1;
    }

    QString type(void) const
    { return "DoubleTagPattern"; }

};

class HtmlPattern : public Pattern
{
public:
    HtmlPattern(const QString &pattern, const std::weak_ptr<Markdown> &md) :
        Pattern(pattern, md)
    {}

    boost::optional<QString> handleMatch(const QRegularExpressionMatch &m)
    {
        QString rawHtml = this->unescape(m.captured(2));
        return this->markdown.lock()->htmlStash.store(rawHtml);
    }

    QString type(void) const
    { return "HtmlPattern"; }

    QString unescape(const QString &text)
    {
        TreeProcessor::StashNodes stash;
        if ( this->markdown.lock()->treeprocessors.exists("inline") ) {
            stash = this->markdown.lock()->treeprocessors["inline"]->stashed_nodes;
        } else {
            return text;
        }
        auto get_stash = [&](const QRegularExpressionMatch &m) -> QString {
            QString id = m.captured(1);
            if ( stash.contains(id) ) {
                boost::optional<QString> str;
                boost::optional<Element> node;
                std::tie(str, node) = stash[id];
                if ( str ) {
                    return "\\" + *str;
                } else {
                    return this->markdown.lock()->serializer(*node);
                }
            }
            return QString();
        };
        return pypp::re::sub(util::INLINE_PLACEHOLDER_RE, get_stash, text);
    }

};

/*!
 * Return a link element from the given match.
 */
class LinkPattern : public Pattern
{
public:
    LinkPattern(const QString& pattern, const std::weak_ptr<Markdown> &md) :
        Pattern(pattern, md)
    {}
    virtual ~LinkPattern(void)
    {}

    virtual Element handleMatch(const ElementTree &, const QRegularExpressionMatch &m)
    {
        Element el = createElement("a");
        el->text = m.captured(2);
        QString title = m.captured(13);
        QString href  = m.captured(9);

        if ( ! href.isEmpty() ) {
            if ( href.startsWith('<') ) {
                href = href.mid(1, href.size()-2);
            }
            el->set("href", this->sanitize_url(this->unescape(href.trimmed())));
        } else {
            el->set("href", QString());
        }

        if ( ! title.isEmpty() ) {
            title = dequote(this->unescape(title));
            el->set("title", title);
        }

        return el;
    }

    /*!
     * Sanitize a url against xss attacks in "safe_mode".
     *
     * Rather than specifically blacklisting `javascript:alert("XSS")` and all
     * its aliases (see <http://ha.ckers.org/xss.html>), we whitelist known
     * safe url formats. Most urls contain a network location, however some
     * are known not to (i.e.: mailto links). Script urls do not contain a
     * location. Additionally, for `javascript:...`, the scheme would be
     * "javascript" but some aliases will appear to `urlparse()` to have no
     * scheme. On top of that relative links (i.e.: "foo/bar.html") have no
     * scheme. Therefore we must check "path", "parameters", "query" and
     * "fragment" for any literal colons. We don't check "scheme" for colons
     * because it *should* never have any and "netloc" must allow the form:
     * `username:password@host:port`.
     *
     */
    QString sanitize_url(QString url)
    {
        QString result = url;
        if ( ! this->markdown.lock()->safeMode() ) {
            //! Return immediately bipassing parsing.
            return result;
        }
        QSet<QString> locless_schemes = {"", "mailto", "news"};
        QSet<QString> allowed_schemes = {"", "mailto", "news", "http", "https", "ftp", "ftps"};

        QUrl qurl(result);
        if ( ! allowed_schemes.contains(qurl.scheme()) ) {
            //! Not a known (allowed) scheme. Not safe.
            return QString();
        }
        if ( qurl.host().isEmpty() && ! locless_schemes.contains(qurl.scheme()) ) {
            //! This should not happen. Treat as suspect.
            return QString();
        }
        //! A colon in "path", "parameters", "query" or "fragment" is suspect.
        if ( qurl.path().contains(":") || qurl.fragment().contains(":") ) {
            return QString();
        }
        for ( const QPair<QString, QString> &item : QUrlQuery(qurl).queryItems() ) {
            if ( item.first.contains(":") || item.second.contains(":") ) {
                return QString();
            }
        }
        //! Url passes all tests. Return url as-is.
        return qurl.toString();

        /*
        int pos = result.indexOf(":");
        if ( pos == -1 ) {
            return QString();
        }
        QString scheme = result.left(pos);
        if ( ! allowed_schemes.contains(scheme) ) {
            //! Not a known (allowed) scheme. Not safe.
            return QString();
        }
        QRegularExpressionMatch m = QRegularExpression("[a-zA-Z][a-zA-Z0-9+\\-.]*://[^/]+/?(.*)").match(result);
        bool ret = m.hasMatch();
        if ( ! ret && ! locless_schemes.contains(scheme) ) {
            //! This should not happen. Treat as suspect.
            return QString();
        }
        QString path;
        if ( ret ) {
            path = m.captured(1);
        } else {
            ++pos;
            path = result.mid(pos);
        }
        if ( path.contains(":") ) {
            return QString();
        }
        return result;
        */
    }

    virtual QString type(void) const
    { return "LinkPattern"; }

};

/*!
 * Return a img element from the given match.
 */
class ImagePattern : public LinkPattern
{
public:
    ImagePattern(const QString &pattern, const std::weak_ptr<Markdown> &md) :
        LinkPattern(pattern, md)
    {}

    Element handleMatch(const ElementTree &, const QRegularExpressionMatch &m)
    {
        std::shared_ptr<Markdown> markdown = this->markdown.lock();

        Element el = createElement("img");
        QString src_parts_source = m.captured(9);
        QStringList src_parts = pypp::split(src_parts_source);
        if ( ! src_parts.isEmpty() ) {
            QString src = src_parts.at(0);
            if ( src.startsWith('<') && src.endsWith('>') ) {
                src = src.mid(1, src.size()-2);
            }
            el->set("src", src);
        } else {
            el->set("src", QString());
        }
        if ( src_parts.size() > 1 ) {
            src_parts.erase(src_parts.begin());
            el->set("title", dequote(this->unescape(src_parts.join(" "))));
        }

        QString truealt;
        if ( markdown->enable_attributes() ) {
            truealt = handleAttributes(m.captured(2), el);
        } else {
            truealt = m.captured(2);
        }

        el->set("alt", this->unescape(truealt));
        return el;
    }

    QString type(void) const
    { return "ImagePattern"; }

};

class ReferencePattern : public LinkPattern
{
public:
    ReferencePattern(const QString &pattern, const std::weak_ptr<Markdown> &md) :
        LinkPattern(pattern, md),
        NEWLINE_CLEANUP_RE("[ ]?\\n", QRegularExpression::MultilineOption)
    {}
    virtual ~ReferencePattern(void)
    {}

    Element handleMatch(const ElementTree &doc, const QRegularExpressionMatch &m)
    {
        std::shared_ptr<Markdown> markdown = this->markdown.lock();

        QString id;
        if ( m.capturedTexts().size() > 8 && ! m.captured(9).isEmpty() ) {
            id = m.captured(9);
        } else {
            //! if we got something like "[Google][]" or "[Goggle]"
            //! we'll use "google" as the id
            id = m.captured(2);
        }
        id = id.toLower();

        //! Clean up linebreaks in id
        id = id.replace(this->NEWLINE_CLEANUP_RE, " ");
        if ( ! markdown->references.contains(id) ) {
            return Element();
        }
        Markdown::ReferenceItem item = markdown->references[id];

        QString text = m.captured(2);
        return this->makeTag(doc, item.first, item.second, text);
    }

    virtual Element makeTag(const ElementTree &, const QString &href, const QString &title, const QString &text)
    {
        Element el = createElement("a");

        el->set("href", this->sanitize_url(href));
        if ( ! title.isEmpty() ) {
            el->set("title", title);
        }

        el->text = text;
        return el;
    }

    virtual QString type(void) const
    { return "ReferencePattern"; }

private:
    QRegularExpression NEWLINE_CLEANUP_RE;

};

/*!
 * Match to a stored reference and return img element.
 */
class ImageReferencePattern : public ReferencePattern
{
public:
    ImageReferencePattern(const QString &pattern, const std::weak_ptr<Markdown> &md) :
        ReferencePattern(pattern, md)
    {}

    Element makeTag(const ElementTree &, const QString &href, const QString &title, const QString &text)
    {
        Element el = createElement("img");

        el->set("src", this->sanitize_url(href));
        if ( ! title.isEmpty() ) {
            el->set("title", title);
        }

        QString text_ = text;
        if ( this->markdown.lock()->enable_attributes() ) {
            text_ = handleAttributes(text, el);
        }

        el->set("alt", this->unescape(text_));
        return el;
    }

    virtual QString type(void) const
    { return "ImageReferencePattern"; }

};

/*!
 * Return a link Element given an autolink (`<http://example/com>`).
 */
class AutolinkPattern : public Pattern
{
public:
    AutolinkPattern(const QString &pattern, const std::weak_ptr<Markdown> &md) :
        Pattern(pattern, md)
    {}

    Element handleMatch(const ElementTree &, const QRegularExpressionMatch &m)
    {
        Element el = createElement("a");
        el->set("href", this->unescape(m.captured(2)));
        el->text = m.captured(2);
        el->atomic = true;
        return el;
    }

    QString type(void) const
    { return "AutolinkPattern"; }

};

/*!
 * Return a mailto link Element given an automail link (`<foo@example.com>`).
 */
class AutomailPattern : public Pattern
{
public:
    AutomailPattern(const QString &pattern, const std::weak_ptr<Markdown> &md) :
        Pattern(pattern, md)
    {}

    Element handleMatch(const ElementTree &, const QRegularExpressionMatch &m)
    {
        Element el = createElement("a");
        QString email = this->unescape(m.captured(2));
        if ( email.startsWith("mailto:") ) {
            email = email.mid(7);
        }

        QString letters;
        for ( const QChar &ch : email ) {
            if ( codepoint2name.contains(ch) ) {
                letters += QString("%1%2;").arg(util::AMP_SUBSTITUTE).arg(codepoint2name[ch]);
            } else {
                letters += QString("%1#%2;").arg(util::AMP_SUBSTITUTE).arg(QString::number(ch.unicode()));
            }
        }
        el->text = letters;
        el->atomic = true;

        QString mailto = "mailto:"+email;
        QString buff;
        for ( int i = 0; i < mailto.size(); ++i ) {
            QChar ch = mailto.at(i);
            buff.append(QString("%1#%2;").arg(util::AMP_SUBSTITUTE, QString::number(ch.unicode())));
        }
        mailto = buff;

        el->set("href", this->unescape(mailto));
        return el;
    }

    QString type(void) const
    { return "AutomailPattern"; }

};

OrderedDictPatterns build_inlinepatterns(const std::shared_ptr<Markdown> &md_instance)
{
    OrderedDictPatterns inlinePatterns;
    inlinePatterns.append("backtick", std::shared_ptr<Pattern>(new BacktickPattern(BACKTICK_RE)));
    inlinePatterns.append("escape", std::shared_ptr<Pattern>(new EscapePattern(ESCAPE_RE, md_instance)));
    inlinePatterns.append("reference", std::shared_ptr<Pattern>(new ReferencePattern(REFERENCE_RE, md_instance)));
    inlinePatterns.append("link", std::shared_ptr<Pattern>(new LinkPattern(LINK_RE, md_instance)));
    inlinePatterns.append("image_link", std::shared_ptr<Pattern>(new ImagePattern(IMAGE_LINK_RE, md_instance)));
    inlinePatterns.append("image_reference", std::shared_ptr<Pattern>(new ImageReferencePattern(IMAGE_REFERENCE_RE, md_instance)));
    inlinePatterns.append("short_reference", std::shared_ptr<Pattern>(new ReferencePattern(SHORT_REF_RE, md_instance)));
    inlinePatterns.append("autolink", std::shared_ptr<Pattern>(new AutolinkPattern(AUTOLINK_RE, md_instance)));
    inlinePatterns.append("automail", std::shared_ptr<Pattern>(new AutomailPattern(AUTOMAIL_RE, md_instance)));
    inlinePatterns.append("linebreak", std::shared_ptr<Pattern>(new SubstituteTagPattern(LINE_BREAK_RE, "br")));
    if ( md_instance->safeMode() != Markdown::escape_mode ) {
        inlinePatterns.append("html", std::shared_ptr<Pattern>(new HtmlPattern(HTML_RE, md_instance)));
    }
    inlinePatterns.append("entity", std::shared_ptr<Pattern>(new HtmlPattern(ENTITY_RE, md_instance)));
    inlinePatterns.append("not_strong", std::shared_ptr<Pattern>(new SimpleTextPattern(NOT_STRONG_RE)));
    inlinePatterns.append("em_strong", std::shared_ptr<Pattern>(new DoubleTagPattern(EM_STRONG_RE, "strong,em")));
    inlinePatterns.append("strong_em", std::shared_ptr<Pattern>(new DoubleTagPattern(STRONG_EM_RE, "em,strong")));
    inlinePatterns.append("strong", std::shared_ptr<Pattern>(new SimpleTagPattern(STRONG_RE, "strong")));
    inlinePatterns.append("emphasis", std::shared_ptr<Pattern>(new SimpleTagPattern(EMPHASIS_RE, "em")));
    if ( md_instance->smart_emphasis() ) {
        inlinePatterns.append("emphasis2", std::shared_ptr<Pattern>(new SimpleTagPattern(SMART_EMPHASIS_RE, "em")));
    } else {
        inlinePatterns.append("emphasis2", std::shared_ptr<Pattern>(new SimpleTagPattern(EMPHASIS_2_RE, "em")));
    }
    return inlinePatterns;
}

} // end of namespace markdown
