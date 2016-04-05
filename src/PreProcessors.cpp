/*
 * PreProcessors.cpp
 *
 *  Created on: 2013/10/25
 *      Author: mugwort_rc
 */

#include "PreProcessors.h"

#include <tuple>

#include <boost/algorithm/string.hpp>

#include "Markdown.h"
#include "util.h"

namespace markdown{

class PreProcessor : public Processor
{
public:
    using Processor::Processor;

	virtual ~PreProcessor(void)
	{}
};

/*!
 * Normalize whitespace for consistant parsing.
 */
class NormalizeWhitespace : public PreProcessor
{
public:
    using PreProcessor::PreProcessor;

	virtual ~NormalizeWhitespace(void)
	{}

    QStringList run(const QStringList &lines)
	{
        std::shared_ptr<Markdown> markdown = this->markdown.lock();

        QString source = lines.join("\n");
        source = source.replace(util::STX, QString());
        source = source.replace(util::ETX, QString());
        source = source.replace("\r\n", "\n");
        source = source.replace("\r", "\n");
        source += "\n\n";
        source = pypp::expandtabs(source, markdown->tab_length());
        source = source.replace(QRegularExpression("(?<=\n) +\n"), "\n");
        return source.split("\n");
	}

};

/*!
 * Remove html blocks from the text and store them for later retrieval.
 */
class HtmlBlockProcessor : public PreProcessor
{
public:
    typedef QMap<QString, QString> Attributes;

public:
    HtmlBlockProcessor(const std::weak_ptr<Markdown> &markdown_instance) :
		PreProcessor(markdown_instance),
        right_tag_patterns({"</%1>", "%1>"}),
        attrs_pattern("\\s+(?<attr>[^>\"'/= ]+)=(?<q>['\"])(?<value>.*?)\\g{q}|\\s+(?<attr1>[^>\"'/= ]+)=(?<value1>[^> ]+)|\\s+(?<attr2>[^>\"'/= ]+)"),
        left_tag_pattern(QString("^<(?<tag>[^> ]+)(?<attrs>(%1)*)\\s*\\/?>?").arg(this->attrs_pattern)),
        attrs_re(this->attrs_pattern),
        left_tag_re(this->left_tag_pattern),
		markdown_in_raw(false)
	{}
	virtual ~HtmlBlockProcessor(void)
	{}

    QStringList run(const QStringList &lines)
	{
        std::shared_ptr<Markdown> markdown = this->markdown.lock();

        QString text = lines.join("\n");
        QStringList new_blocks;
        QStringList texts;
		// python str.rsplit()
		while ( true ) {
            int i = text.lastIndexOf("\n\n");
            if ( i == -1 ) {
				texts.push_front(text);
				break;
			}
            int count = text.size() - (i + 2);
            texts.push_front(text.mid(i+2, count));
            text = text.left(i);
		}
        QStringList items;
        QString left_tag;
        QString right_tag;
		Attributes attrs;
		int left_index;
        int data_index;
		bool in_tag = false;  //!< flag
		while ( texts.size() > 0 ) {
            QString block = texts.front();
            if ( block[0] == '\n' ) {
                block = block.mid(1);
			}
			texts.pop_front();

            if ( block[0] == '\n' ) {
                block = block.mid(1);
			}

			if ( ! in_tag ) {
                if ( block[0] == '<' && block.trimmed().size() > 1 ) {
                    if ( block.mid(1, 3) == "!--" ) {
						//! is a comment block
                        left_tag   = "--";
						left_index = 2;
						attrs      = Attributes();
					} else {
                        std::tie(left_tag, left_index, attrs) = this->get_left_tag(block);
					}
                    std::tie(right_tag, data_index) = this->get_right_tag(left_tag, left_index, block);

					//! keep checking conditions below and maybe just append

                    if ( data_index < block.size() && ( util::isBlockLevel(left_tag) || left_tag == "--" ) ) {
                        texts.push_front(block.mid(data_index));
                        block = block.left(data_index);
					}

                    QChar ch = block[1];
                    if ( ! util::isBlockLevel(left_tag) || ( ch == '!' || ch == '?' || ch == '@' || ch == '%' ) ) {
						new_blocks.push_back(block);
						continue;
					}

					if ( this->is_oneliner(left_tag) ) {
                        new_blocks.push_back(block.trimmed());
						continue;
					}

                    QString buff = pypp::rstrip(block);
                    if ( buff.endsWith('>') && this->equal_tags(left_tag, right_tag) ) {
                        if ( this->markdown_in_raw && attrs.contains("markdown") ) {
                            QString start = block.left(left_index).replace(QRegularExpression("\\smarkdown(=['\"]?[^> ]*['\"]?)?"), QString());
							int begin = block.size()-right_tag.size()-2;
                            QString end = block.mid(begin);
                            block = block.mid(left_index, begin-left_index);
                            new_blocks.push_back(markdown->htmlStash.store(start));
							new_blocks.push_back(block);
                            new_blocks.push_back(markdown->htmlStash.store(end));
						} else {
                            new_blocks.push_back(markdown->htmlStash.store(block.trimmed()));
						}
						continue;
					} else {
						//! if is block level tag and is not complete

                        QString buff = pypp::rstrip(block);
                        if ( ( util::isBlockLevel(left_tag) || left_tag == "--" ) && ! buff.endsWith('>') ) {
                            items.push_back(block.trimmed());
							in_tag = true;
						} else {
                            new_blocks.push_back(markdown->htmlStash.store(block.trimmed()));
						}

						continue;
					}
				}

				new_blocks.push_back(block);

			} else {
				items.push_back(block);

                std::tie(right_tag, data_index) = this->get_right_tag(left_tag, 0, block);

				if ( this->equal_tags(left_tag, right_tag) ) {
					//! if find closing tag

					if ( data_index < block.size() ) {
						//! we have more text after right_tag
                        items.back() = block.left(data_index);
                        texts.push_front(block.mid(data_index));
					}

					in_tag = false;
                    if (this->markdown_in_raw && attrs.contains("markdown") ) {
                        QString start = items.front().left(left_index).replace(QRegularExpression("\\smarkdown(=['\"]?[^> ]*['\"]?)?"), QString());
                        items.front() = items.front().mid(left_index);
						int begin = items.back().size()-right_tag.size()-2;
                        QString end = items.back().mid(begin);
                        items.back() = items.back().left(begin);
                        new_blocks.push_back(markdown->htmlStash.store(start));
                        for ( const QString &item : items ) {
							new_blocks.push_back(item);
						}
                        new_blocks.push_back(markdown->htmlStash.store(end));
					} else {
                        new_blocks.push_back(items.join("\n\n"));
					}
                    items = QStringList();
				}
			}
        }
		if ( items.size() > 0 ) {
            if ( this->markdown_in_raw && attrs.contains("markdown") ) {
                QString start = items.front().left(left_index).replace(QRegularExpression("\\smarkdown(=['\"]?[^> ]*['\"]?)?"), QString());
                items.front() = items.front().mid(left_index);
				int begin = items.back().size()-right_tag.size()-2;
                QString end = items.back().mid(begin);
                items.back() = items.back().left(begin);
                new_blocks.push_back(markdown->htmlStash.store(start));
                for ( const QString &item : items ) {
					new_blocks.push_back(item);
				}
                new_blocks.push_back(markdown->htmlStash.store(end));
			} else {
                new_blocks.push_back(items.join("\n\n"));
            }
            new_blocks.push_back("\n");
		}
        QString new_text = new_blocks.join("\n\n");
        return new_text.split("\n");
	}

private:
    std::tuple<QString, int, Attributes> get_left_tag(const QString &block)
	{
        QRegularExpressionMatch m = this->left_tag_re.match(block);
        if ( m.hasMatch() ) {
            QString tag = m.captured("tag");
            QString raw_attrs = m.captured("attrs");
			Attributes attrs;
            if ( ! raw_attrs.isEmpty() ) {
                QRegularExpressionMatchIterator it = this->attrs_re.globalMatch(raw_attrs);
                while ( it.hasNext() ) {
                    QRegularExpressionMatch ma = it.next();
                    if ( ! ma.captured("attr").isEmpty() ) {
                        QString attr = ma.captured("attr").trimmed();
                        if ( ! ma.captured("value").isEmpty() ) {
                            attrs[attr] = ma.captured("value");
						} else {
                            attrs[attr] = QString();
						}
                    } else if ( ! ma.captured("attr1").isEmpty() ) {
                        QString attr1 = ma.captured("attr1").trimmed();
                        if ( ! ma.captured("value1").isEmpty() ) {
                            attrs[attr1] = ma.captured("value1");
						} else {
                            attrs[attr1] = QString();
						}
                    } else if ( ! ma.captured("attr2").isEmpty() ) {
                        QString attr2 = ma.captured("attr2").trimmed();
                        attrs[attr2] = QString();
					}
				}
            }
            return std::make_tuple(tag, m.captured().size(), attrs);
		} else {
            QString tag = block.mid(1, block.indexOf('>')-1).toLower();
            return std::make_tuple(tag, tag.size()+2, Attributes());
		}
	}
    int recursive_tagfind(const QString &ltag, const QString &rtag, int start_index, const QString &block)
	{
		while ( true ) {
            int i = block.indexOf(rtag, start_index);
            if ( i == -1 ) {
				return -1;
			}
            int j = block.indexOf(ltag, start_index);
			//! if no ltag, or rtag found before another ltag, return index
            if ( j > i || j == -1 ) {
				return i + rtag.size();
			}
			//! another ltag found before rtag, use end of ltag as starting
			//! point and search again
            j = block.indexOf('>', j);
			start_index = this->recursive_tagfind(ltag, rtag, j+1, block);
			if ( start_index == -1 ) {
				//! HTML potentially malformed- ltag has no corresponding
				//! rtag
				return -1;
			}
		}
	}
    std::tuple<QString, int> get_right_tag(const QString &left_tag, int left_index, const QString &block)
	{
        for ( const QString &p : this->right_tag_patterns ) {
            QString tag = p.arg(left_tag);
            int i = this->recursive_tagfind(QString("<%1").arg(left_tag), tag, left_index, block);
			if ( i > 2 ) {
                tag = pypp::lstrip(tag, [](const QChar &ch) -> bool { return ch == '<'; });
                tag = pypp::rstrip(tag, [](const QChar &ch) -> bool { return ch == '>'; });
                return std::make_tuple(tag, i);
			}
		}
        QString result = pypp::rstrip(block);
		int begin = result.size()-left_index;
		int count = result.size()-begin-1;
        result = result.mid(result.size()-left_index, count).toLower();
        return std::make_tuple(result, block.size());
	}
    bool equal_tags(const QString &left_tag, const QString &right_tag)
	{
        QChar ch = left_tag[0];
        if ( ch == '?' || ch == '@' || ch == '%' ) {  //!< handle PHP, etc.
			return true;
		}
        if ( ("/"+left_tag) == right_tag ) {
			return true;
		}
        if ( right_tag == "--" && left_tag == "--" ) {
			return true;
        } else if ( left_tag == right_tag.mid(1)
                      && right_tag.startsWith('/') ) {
			return true;
		} else {
			return false;
		}
	}
    bool is_oneliner(const QString &tag)
	{
        return tag == "hr" || tag == "hr/";
	}

private:
    QStringList        right_tag_patterns;
    QString            attrs_pattern;
    QString            left_tag_pattern;
    QRegularExpression attrs_re;
    QRegularExpression left_tag_re;
    bool               markdown_in_raw;

};

/*!
 * Remove reference definitions from text and store for later use.
 */
class ReferencePreprocessor : public PreProcessor
{
public:
    ReferencePreprocessor(const std::weak_ptr<Markdown> &markdown_instance) :
		PreProcessor(markdown_instance),
        TITLE("[ ]*(\\\"(.*)\\\"|\\'(.*)\\'|\\((.*)\\))[ ]*"),
        RE(QString("^[ ]{0,3}\\[([^\\]]*)\\]:\\s*([^ ]*)[ ]*(%1)?$").arg(this->TITLE), QRegularExpression::DotMatchesEverythingOption),
        TITLE_RE(QString("^%1$").arg(this->TITLE))
	{}

    QStringList run(const QStringList &lines)
	{
        std::shared_ptr<Markdown> markdown = this->markdown.lock();

        QStringList buffer = lines;
        QStringList new_text;
		while ( buffer.size() > 0 ) {
            QString line = buffer.front();
			buffer.pop_front();
            QRegularExpressionMatch m = this->RE.match(line);
            if ( m.hasMatch() ) {
                QString id = m.captured(1).trimmed().toLower();
                QString link = m.captured(2);
                link = pypp::lstrip(link, [](const QChar &ch) -> bool { return ch == '<'; });
                link = pypp::rstrip(link, [](const QChar &ch) -> bool { return ch == '>'; });
                QString t;
				for ( int i = 5; i <= 7; ++i ) {
                    t = m.captured(i);
					if ( t.size() > 0 ) {
						break;
					}
				}
                if ( ! ( m.captured(5).size() > 0 || m.captured(6).size() > 0 || m.captured(7).size() > 0 ) ) {
					//! Check next line for title
                    QRegularExpressionMatch tm = this->TITLE_RE.match(buffer.front());
                    if ( tm.hasMatch() ) {
						buffer.pop_front();
						for ( int i = 2; i <= 4; ++i ) {
                            t = tm.captured(i);
							if ( t.size() > 0 ) {
								break;
							}
						}
					}
				}
                markdown->references[id] = Markdown::ReferenceItem(link, t);
			} else {
				new_text.push_back(line);
			}
		}
		return new_text;
	}

private:
    const QString TITLE;
    const QRegularExpression RE;
    const QRegularExpression TITLE_RE;

};

OrderedDictProcessors build_preprocessors(const std::shared_ptr<Markdown> &md_instance)
{
	OrderedDictProcessors preProcessors;
    preProcessors.append("normalize_whitespace", std::shared_ptr<Processor>(new NormalizeWhitespace(md_instance)));
	if ( md_instance->safeMode() != Markdown::escape_mode ) {
        preProcessors.append("html_block", std::shared_ptr<Processor>(new HtmlBlockProcessor(md_instance)));
	}
    preProcessors.append("reference", std::shared_ptr<Processor>(new ReferencePreprocessor(md_instance)));
	return preProcessors;
}

} // end of namespace markdown
