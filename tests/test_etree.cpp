#include "test_etree.h"

#include <QTest>

#include "ElementTree.hpp"
#include "Serializers.h"

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
    markdown::Element elem = markdown::createElement("elem");
    QCOMPARE(markdown::to_xhtml_string(elem), QString("<elem></elem>"));
}

void TestEtree::test_toString()
{
    //QCOMPARE(this->root.toString(), QString());
}

void TestEtree::test_element_names()
{
    markdown::Element el;

    el = markdown::createElement("name");
    QCOMPARE(el->tag, QString("name"));

    //el = markdown::Element(this->root, "{}name");
    //QCOMPARE(el.getTagName(), QString("name"));
}

