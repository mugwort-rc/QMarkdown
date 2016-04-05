/*
 * ElementTree.hpp
 *
 *  Created on: 2013/11/09
 *      Author: mugwort_rc
 */

#ifndef ELEMENTTREE_HPP_
#define ELEMENTTREE_HPP_

#include "pypp.hpp"

namespace markdown{

using pypp::xml::etree::ElementList_t;

using pypp::xml::etree::ElementTree;

typedef pypp::xml::etree::ElementPtr Element;

inline Element createElement(const pypp::str &tag)
{
    return pypp::xml::etree::Element::makeelement(tag);
}

inline Element createSubElement(const Element &element, const pypp::str &tag)
{
    return pypp::xml::etree::SubElement(element, tag);
}

} // end of namespace markdown

#endif // ELEMENTTREE_HPP_
