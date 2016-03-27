#include "test_pypp.h"

#include <QTest>

TestPypp::TestPypp()
{}

TestPypp::~TestPypp()
{}

void TestPypp::initTestCase()
{}

void TestPypp::cleanupTestCase()
{}

void TestPypp::init()
{}

void TestPypp::cleanup()
{}

void TestPypp::test_re_sub()
{
    auto repl = [](const QRegularExpressionMatch &) -> QString { return QString(); };
    QCOMPARE(pypp::re::sub(QRegularExpression("\\d"), repl, "123, 456"), QString(", "));
}


