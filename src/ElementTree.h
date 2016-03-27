/*
 * ElementTree.h
 *
 *  Created on: 2013/11/09
 *      Author: mugwort_rc
 */

#ifndef ELEMENTTREE_H_
#define ELEMENTTREE_H_

#include <memory>

#include <QMap>
#include <QString>

namespace markdown{

class ElementImpl;      //!< Pimpl
class ElementTreeImpl;  //!< PImpl

class Element;  //!< forward declaration for ElementTree

class ElementTree
{
    friend class Element;

private:
    typedef std::shared_ptr<ElementTreeImpl> Impl;

public:
    ElementTree();
    ElementTree(const QString &root);
    ElementTree(const Impl &impl);
    ElementTree(const ElementTree &copy);
    ElementTree(ElementTree &&move);
    ElementTree &operator =(const ElementTree &rhs);

    void _setroot(const Element &elem);
    Element getroot() const;

    QString toString() const;

public:
    static ElementTree InvalidElementTree;

private:
    Impl _impl;

};

class Element
{
    friend class ElementTree;

public:
    typedef QList<Element> List;
    typedef QMap<QString, QString> Attributes;

private:
    typedef std::shared_ptr<ElementImpl> Impl;

public:
    Element();
    Element(const ElementTree &doc);
    Element(const ElementTree &doc, const QString &name);
    /*!
     * @code:
     * Element parent;
     * Element child(parent, L"child");  //!< @note: not appended
     *
     * parent.append(child);
     *
     * // or
     *
     * parent.insertBefore(child, parent.getFirstElementChild());
     *
     * @endcode
     */
    Element(const Element &parent, const QString &name);
    Element(const Element &copy);
    Element(Element &&move);

    Element &operator =(const Element &rhs);

    bool operator ==(const Element &rhs) const;
    bool operator !=(const Element &rhs) const;

    ElementTree getOwnerDocument(void) const;

    List child(void) const;
    List getElementsByTagName(const QString &name) const;
    Element getFirstElementChild(void) const;
    Element getLastElementChild(void) const;
    Element getNextElementSibling(void) const;

    bool isNull(void) const;
    QString getTagName(void) const;

    void setAttribute(const QString &key, const QString &val);
    Attributes getAttributes(void) const;

    QString getNamespaceURI(void) const;
    QString getTextContent(void) const;

    bool hasText(void) const;
    bool hasTail(void) const;

    void removeText(void);
    void removeTail(void);

    void setText(const QString &text);
    void setTail(const QString &tail);

    QString text(void) const;
    QString tail(void) const;

    void append(Element &child);
    void insertBefore(Element &child, const Element &ref);
    void remove(Element &child);

public:
    static Element InvalidElement;

private:
    Element(const Impl &elem);
    Impl import(Impl &target, bool deep=true);
    void initialize(void);

private:
    Impl _impl;

};

} // end of namespace markdown

#endif // ELEMENTTREE_H_
