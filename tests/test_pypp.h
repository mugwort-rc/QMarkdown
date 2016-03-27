#ifndef TEST_PYPP_H
#define TEST_PYPP_H

#include <QObject>

#include "pypp.hpp"

class TestPypp : public QObject
{
    Q_OBJECT
public:
    TestPypp();
    ~TestPypp();

private slots:
    void initTestCase();
    void cleanupTestCase();

    void init();
    void cleanup();

    void test_re_sub();

};

#endif // TEST_PYPP_H
