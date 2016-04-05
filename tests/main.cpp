#include <QtTest>
#include <QCoreApplication>

#include "xerces.hpp"

#include "test_pypp.h"
#include "test_etree.h"
#include "test_apis.h"
#include "test_basic.h"

int main(int argc, char *argv[]) {
    XercesInitializer xerces;
    Q_UNUSED(xerces);

    QCoreApplication app(argc, argv);
    Q_UNUSED(app)

    typedef QSharedPointer<QObject> Test;
    QList<Test> tests = {
        Test(new TestPypp()),
        Test(new TestEtree()),
        Test(new TestMarkdownBasics()),
        Test(new TestBlockParser()),
        Test(new TestBlockParserState()),
        Test(new TestHtmlStash()),
        Test(new TestOrderedDict()),
        Test(new TestInlinePattern()),
        Test(new TestTreeProcessor()),
        Test(new TestBasic()),
    };

    for ( QSharedPointer<QObject> &test : tests ) {
        if ( int ret = QTest::qExec(test.data(), argc, argv) != 0 ) {
            return ret;
        }
    }

    return 0;
}

