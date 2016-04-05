#include "elementtree.h"

namespace pypp {

namespace xml {

namespace etree {

ElementPtr SimpleElementPath::find(const ElementPtr &element, const pypp::str &tag, const Namespaces_t &)
{
    for ( const ElementPtr &elem : element->child() ) {
        if ( elem->tag == tag ) {
            return elem;
        }
    }
    return ElementPtr();
}

pypp::str SimpleElementPath::findtext(const ElementPtr &element, const pypp::str &tag, const pypp::str &default_, const Namespaces_t &)
{
    ElementPtr elem = SimpleElementPath::find(element, tag);
    if ( ! elem ) {
        return default_;
    }
    return elem->text;
}

ElementList_t SimpleElementPath::iterfind(const ElementPtr &element, const pypp::str &tag, const Namespaces_t &)
{
    ElementList_t result;
    if ( tag.startsWith(".//") ) {
        for ( const ElementPtr &elem : element->iter(tag.mid(3)) ) {
            result.append(elem);
        }
    }
    for ( const ElementPtr &elem : element->child() ) {
        if ( elem->tag == tag ) {
            result.append(elem);
        }
    }
    return result;
}

ElementList_t SimpleElementPath::findall(const ElementPtr &element, const pypp::str &tag, const Namespaces_t &namespaces)
{
    return SimpleElementPath::iterfind(element, tag, namespaces);
}

} // namespace etree

} // namespace xml

} // namespace pypp
