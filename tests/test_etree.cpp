#include "test_etree.h"

#include <QTest>

#include "ElementTree.h"

TestEtree::TestEtree() :
    root()
{}

TestEtree::~TestEtree()
{}

void TestEtree::initTestCase()
{}

void TestEtree::cleanupTestCase()
{}

void TestEtree::init()
{
    this->root = markdown::ElementTree();
}

void TestEtree::cleanup()
{}

void TestEtree::test_Element()
{
    markdown::ElementTree tree;
    markdown::Element elem(tree, "elem");
    tree._setroot(elem);
    QCOMPARE(tree.toString(), QString("<elem/>"));
}

void TestEtree::test_toString()
{
    QCOMPARE(this->root.toString(), QString());
}

void TestEtree::test_element_names()
{
    markdown::Element el;

    el = markdown::Element(this->root, "name");
    QCOMPARE(el.getTagName(), QString("name"));

    //el = markdown::Element(this->root, "{}name");
    //QCOMPARE(el.getTagName(), QString("name"));
}

