/*
 * tables.cpp
 *
 *  Created on: 2013/11/04
 *      Author: mugwort_rc
 */

#include "tables.h"

#include <QString>
#include <QStringList>

#include "../Markdown.h"
#include "../BlockParser.h"
#include "../BlockProcessors.h"

namespace markdown{

/*!
 * Process Tables.
 */
class TableProcessor : public BlockProcessor
{
public:
    TableProcessor(const std::weak_ptr<BlockParser> &parser) :
        BlockProcessor(parser),
        CHECK_CHARS({'|', ':', '-'})
    {}

    bool test(Element &, const QString &block)
    {
        QStringList rows = block.split("\n");
        return rows.size() > 2
                && rows[0].contains(L'|')
                && rows[1].contains(L'|')
                && rows[1].contains(L'-')
                && this->CHECK_CHARS.contains(rows[1].trimmed().at(0));
    }

    /*!
     * Parse a table block and build table.
     */
    void run(Element &parent, QStringList &blocks)
    {
        QString blocksTmp = blocks.front();
        blocks.pop_front();
        QStringList block = blocksTmp.split("\n");
        QString header = block[0].trimmed();
        QString separator = block[1].trimmed();
        QStringList rows;
        int counter = 0;
        for ( const QString &row : block ) {
            if ( ++counter < 3 ) {
                continue;
            }
            rows.push_back(row);
        }
        //! Get format type (bordered by pipes or not)
        bool border = false;
        if ( header.at(0) == '|' ) {
            border = true;
        }
        //! Get alignment of columns
        QList<boost::optional<QString>> align;
        for ( const QString &c : this->split_row(separator, border) ) {
            if ( c.startsWith(':') && c.endsWith(':') ) {
                align.push_back(QString("center"));
            } else if ( c.startsWith(':') ) {
                align.push_back(QString("left"));
            } else if ( c.endsWith(':') ) {
                align.push_back(QString("right"));
            } else {
                align.push_back(boost::none);
            }
        }
        //! Build table
        Element table = createSubElement(parent, "table");
        Element thead = createSubElement(table, "thead");
        this->build_row(header, thead, align, border);
        Element tbody = createSubElement(table, "tbody");
        for ( const QString &row : rows ) {
            this->build_row(row.trimmed(), tbody, align, border);
        }
    }

private:
    /*!
     * Given a row of text, build table cells.
     */
    void build_row(const QString &row, Element &parent, const QList<boost::optional<QString>> &align, bool border)
    {
        Element tr = createSubElement(parent, "tr");
        QString tag = "td";
        if ( parent->tag == "thead" ) {
            tag = "th";
        }
        QStringList cells = this->split_row(row, border);
        //! We use align here rather than cells to ensure every row
        //! contains the same number of columns.
        int i = 0;
        for ( boost::optional<QString> a : align ) {
            Element c = createSubElement(tr, tag);
            if ( cells.size() > i ) {
                QString cell = cells.at(i).trimmed();
                c->text = cell;
            } else {
                c->text = QString();
            }
            if ( a ) {
                c->set("align", *a);
            }
            ++i;
        }
    }
    /*!
     * split a row of text into list of cells.
     */
    QStringList split_row(const QString &row, bool border)
    {
        QString tmp = row;
        if ( border ) {
            if ( tmp.startsWith('|') ) {
                tmp = tmp.mid(1);
            }
            if ( tmp.endsWith('|') ) {
                tmp = tmp.left(tmp.size()-1);
            }
        }
        return tmp.split("|");
    }

private:
    QSet<QChar> CHECK_CHARS;

};

TableExtension::TableExtension() :
    Extension()
{}

void TableExtension::extendMarkdown(const std::shared_ptr<Markdown> &md/*, md_globals*/)
{
    md->parser->blockprocessors.add("table", std::shared_ptr<BlockProcessor>(new TableProcessor(md->parser)), "<hashheader");
}

Extension::Ptr TableExtension::generate(void)
{
    return std::shared_ptr<Extension>(new TableExtension);
}

} // end of namespace markdown
