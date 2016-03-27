#ifndef TEST_ETREE_H
#define TEST_ETREE_H

#include <QObject>

#include "ElementTree.h"

class TestEtree : public QObject
{
    Q_OBJECT
public:
    TestEtree();
    ~TestEtree();

private slots:
    void initTestCase();
    void cleanupTestCase();

    void init();
    void cleanup();

    void test_Element();
    void test_toString();

    void test_element_names();

private:
    markdown::ElementTree root;

};

#endif // TEST_ETREE_H
