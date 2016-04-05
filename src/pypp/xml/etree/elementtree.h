#ifndef PYPP_ELEMENTTREE_H
#define PYPP_ELEMENTTREE_H

#include <memory>

#include <QList>
#include <QMap>
#include <QPair>

#include "../../str.hpp"

namespace pypp {

namespace xml {

namespace etree {

class Element;
typedef std::shared_ptr<Element> ElementPtr;
typedef QList<ElementPtr> ElementList_t;
typedef QMap<pypp::str, pypp::str> Namespaces_t;
typedef QPair<pypp::str, pypp::str> Item_t;
typedef QList<Item_t> ItemList_t;


class SimpleElementPath
{
public:
    static ElementPtr find(const ElementPtr &element, const pypp::str &tag, const Namespaces_t &namespaces=Namespaces_t());

    static pypp::str findtext(const ElementPtr &element, const pypp::str &tag, const pypp::str &default_, const Namespaces_t &namespaces=Namespaces_t());

    static ElementList_t iterfind(const ElementPtr &element, const pypp::str &tag, const Namespaces_t &namespaces=Namespaces_t());

    static ElementList_t findall(const ElementPtr &element, const pypp::str &tag, const Namespaces_t &namespaces=Namespaces_t());

};

typedef SimpleElementPath ElementPath;


class Element : public std::enable_shared_from_this<Element>
{
public:
    typedef QMap<pypp::str, pypp::str> Attribute_t;

    /*!
      (Attribute) Element tag.
     */
    pypp::str tag;

    /*!
      (Attribute) Element attribute dictionary.  Where possible, use
      {@link #Element.get},
      {@link #Element.set},
      {@link #Element.keys}, and
      {@link #Element.items} to access
      element attributes.
     */
    Attribute_t attrib;

    /*!
      (Attribute) Text before first subelement.  This is either a
      string or the value None.  Note that if there was no text, this
      attribute may be either None or an empty string, depending on
      the parser.
     */
    pypp::str text;

    /*!
      (Attribute) Text after this element's end tag, but before the
      next sibling element's start tag.  This is either a string or
      the value None.  Note that if there was no text, this attribute
      may be either None or an empty string, depending on the parser.
     */
    pypp::str tail;

    /*!
      constructor
     */
    Element(const pypp::str &tag, const Attribute_t &attrib=Attribute_t()) :
        tag(tag),
        attrib(attrib),
        text(),
        tail(),

        atomic(false),

        _children()
    {}
    virtual ~Element()
    {}

    /*!
      Creates a new element object of the same type as this element.

      @param tag Element tag.
      @param attrib Element attributes, given as a dictionary.
      @return A new element instance.
     */
    static ElementPtr makeelement(const pypp::str &tag, const Attribute_t &attrib=Attribute_t())
    {
        return ElementPtr(new Element(tag, attrib));
    }

    /*!
      (Experimental) Copies the current element.  This creates a
      shallow copy; subelements will be shared with the original tree.

      @return A new element instance.
     */
    ElementPtr copy() const
    {
        ElementPtr elem = Element::makeelement(this->tag, this->attrib);
        elem->text = this->text;
        elem->tail = this->tail;
        elem->_children = this->_children;
        return elem;
    }

    /*!
      Returns the number of subelements.  Note that this only counts
      full elements; to check if there's any content in an element, you
      have to check both the length and the <b>text</b> attribute.

      @return The number of subelements.
     */
    int size() const
    {
        return this->_children.size();
    }

    /*!
      Returns the given subelement, by index.

      @param index What subelement to return.
      @return The given subelement.
      @exception IndexError If the given element does not exist.
     */
    ElementPtr &operator [](int index)
    {
        return this->_children[index];
    }
    const ElementPtr &operator [](int index) const
    {
        return this->_children.at(index);
    }

    /*!
      Deletes the given subelement, by index.

      @param index What subelement to delete.
      @exception IndexError If the given element does not exist.
     */
    void removeAt(int index)
    {
        this->_children.removeAt(index);
    }

    /*!
      Adds a subelement to the end of this element.  In document order,
      the new element will appear after the last existing subelement (or
      directly after the text, if it's the first subelement), but before
      the end tag for this element.

      @param element The element to add.
     */
    void append(const ElementPtr &element)
    {
        this->_children.append(element);
    }

    /*!
      Appends subelements from a sequence.

      @param elements A sequence object with zero or more elements.
      @since 1.3
     */
    void extend(const QList<ElementPtr> &elements)
    {
        this->_children.append(elements);
    }

    /*!
      Inserts a subelement at the given position in this element.

      @param index Where to insert the new subelement.
     */
    void insert(int index, const ElementPtr &element)
    {
        this->_children.insert(index, element);
    }

    /*!
      Removes a matching subelement.  Unlike the <b>find</b> methods,
      this method compares elements based on identity, not on tag
      value or contents.  To remove subelements by other means, the
      easiest way is often to use a list comprehension to select what
      elements to keep, and use slice assignment to update the parent
      element.

      @param element What element to remove.
      @exception ValueError If a matching element could not be found.
     */
    void remove(const ElementPtr &element)
    {
        this->_children.removeOne(element);
    }

    /*!
      Finds the first matching subelement, by tag name or path.

      @param path What element to look for.
      @keyparam namespaces Optional namespace prefix map.
      @return The first matching element, or None if no element was found.
      @defreturn Element or None
     */
    ElementPtr find(const pypp::str &path, const Namespaces_t &namespaces=Namespaces_t())
    {
        return ElementPath::find(this->shared_from_this(), path, namespaces);
    }

    /*!
      Finds text for the first matching subelement, by tag name or path.

      @param path What element to look for.
      @param default What to return if the element was not found.
      @keyparam namespaces Optional namespace prefix map.
      @return The text content of the first matching element, or the
          default value no element was found.  Note that if the element
          is found, but has no text content, this method returns an
          empty string.
      @defreturn string
     */
    pypp::str findtext(const pypp::str &path, const pypp::str &default_=pypp::str(), const Namespaces_t &namespaces=Namespaces_t())
    {
        return ElementPath::findtext(this->shared_from_this(), path, default_, namespaces);
    }

    /*!
      Finds all matching subelements, by tag name or path.

      @param path What element to look for.
      @keyparam namespaces Optional namespace prefix map.
      @return A list or other sequence containing all matching elements,
         in document order.
      @defreturn list of Element instances
     */
    ElementList_t findall(const pypp::str &path, const Namespaces_t &namespaces=Namespaces_t())
    {
        return ElementPath::findall(this->shared_from_this(), path, namespaces);
    }

    /*!
      Finds all matching subelements, by tag name or path.

      @param path What element to look for.
      @keyparam namespaces Optional namespace prefix map.
      @return An iterator or sequence containing all matching elements,
         in document order.
      @defreturn a generated sequence of Element instances
     */
    ElementList_t iterfind(const pypp::str &path, const Namespaces_t &namespaces=Namespaces_t())
    {
        return ElementPath::iterfind(this->shared_from_this(), path, namespaces);
    }

    /*!
      Resets an element.  This function removes all subelements, clears
      all attributes, and sets the <b>text</b> and <b>tail</b> attributes
      to None.
     */
    void clear()
    {
        this->attrib.clear();
        this->_children.clear();
        this->text.clear();
        this->tail.clear();
    }

    /*!
      Gets an element attribute.  Equivalent to <b>attrib.get</b>, but
      some implementations may handle this a bit more efficiently.

      @param key What attribute to look for.
      @param default What to return if the attribute was not found.
      @return The attribute value, or the default value, if the
          attribute was not found.
      @defreturn string or None
     */
    pypp::str get(const pypp::str &key, const pypp::str &default_=pypp::str()) const
    {
        if ( this->attrib.contains(key) ) {
            return this->attrib[key];
        }
        return default_;
    }

    /*!
      Sets an element attribute.  Equivalent to <b>attrib[key] = value</b>,
      but some implementations may handle this a bit more efficiently.

      @param key What attribute to set.
      @param value The attribute value.
     */
    void set(const pypp::str &key, const pypp::str &value)
    {
        this->attrib[key] = value;
    }

    /*!
      Gets a list of attribute names.  The names are returned in an
      arbitrary order (just like for an ordinary Python dictionary).
      Equivalent to <b>attrib.keys()</b>.

      @return A list of element attribute names.
      @defreturn list of strings
     */
    QStringList keys() const
    {
        return this->attrib.keys();
    }

    /*!
      Gets element attributes, as a sequence.  The attributes are
      returned in an arbitrary order.  Equivalent to <b>attrib.items()</b>.

      @return A list of (name, value) tuples for all attributes.
      @defreturn list of (string, string) tuples
     */
    ItemList_t items() const
    {
        ItemList_t result;
        for ( auto it = this->attrib.cbegin(); it != this->attrib.cend(); ++it ) {
            result.append(Item_t(it.key(), it.value()));
        }
        return result;
    }

    ElementList_t iter(pypp::str tag=pypp::str())
    {
        if ( tag == "*" ) {
            tag = "";
        }
        ElementList_t result;
        if ( tag.isEmpty() || this->tag == tag ) {
            result.append(this->shared_from_this());
        }
        for ( const ElementPtr &elem : this->_children ) {
            for ( const ElementPtr e : elem->iter(tag) ) {
                result.append(e);
            }
        }
        return result;
    }

    ElementList_t child() const
    {
        return this->_children;
    }



    bool atomic;

    ElementPtr getFirstElementChild() const
    {
        return this->operator [](0);
    }

    ElementPtr getLastElementChild() const
    {
        return this->operator [](this->size()-1);
    }

    bool hasText() const
    {
        return ! this->text.isEmpty();
    }
    bool hasTail() const
    {
        return ! this->tail.isEmpty();
    }

private:
    ElementList_t _children;

};

inline ElementPtr SubElement(const ElementPtr &parent, const pypp::str &tag, const Namespaces_t &attrib=Namespaces_t())
{
    ElementPtr element = Element::makeelement(tag, attrib);
    parent->append(element);
    return element;
}

class ElementTree
{
public:
    ElementTree(const ElementPtr &element=ElementPtr()) :
        _root(element)
    {}

    ElementPtr getroot() const
    {
        return this->_root;
    }
    void _setroot(const ElementPtr &element)
    {
        this->_root = element;
    }

    ElementList_t iter(const pypp::str &tag=pypp::str())
    {
        return this->_root->iter(tag);
    }

private:
    ElementPtr _root;

};

} // namespace etree

} // namespace xml

} // namespace pypp

#endif // PYPP_ELEMENTTREE_H
