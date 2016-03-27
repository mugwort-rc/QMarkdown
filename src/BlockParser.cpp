/*
 * BlockParser.cpp
 *
 *  Created on: 2013/10/27
 *      Author: mugwort_rc
 */

#include "BlockParser.h"

#include "Markdown.h"

namespace markdown{

BlockParser::BlockParser(const std::weak_ptr<Markdown> &markdown) :
	markdown(markdown),
    blockprocessors(), root(ElementTree::InvalidElementTree)
{}

ElementTree BlockParser::parseDocument(const QStringList &lines)
{
    std::shared_ptr<Markdown> markdown = this->markdown.lock();

    this->root = ElementTree(markdown->doc_tag());
    Element tmp(this->root);
    this->parseChunk(tmp, lines.join("\n"));
	return this->root;
}

void BlockParser::parseChunk(Element &parent, const QString &text)
{
    QStringList buffer = text.split(QRegularExpression("\n\n"));
	this->parseBlocks(parent, buffer);
}

void BlockParser::parseBlocks(Element &parent, QStringList &blocks)
{
	while ( blocks.size() > 0 ) {
        for (OrderedDictBlockProcessors::ValueType processor : this->blockprocessors.toList()) {
			if ( processor->test(parent, blocks.front()) ) {
				processor->run(parent, blocks);
				break;
			}
		}
	}
}

} // end of namespace markdown
