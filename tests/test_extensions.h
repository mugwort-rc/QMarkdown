#ifndef TEST_EXTENSIONS_H
#define TEST_EXTENSIONS_H

#include <QObject>

#include "Markdown.h"

class TestExtensions : public QObject
{
    Q_OBJECT
public:
    TestExtensions();
    ~TestExtensions();

private slots:
    void initTestCase();
    void cleanupTestCase();

    void init();
    void cleanup();

    // extra
    void tables();

};

#endif // TEST_EXTENSIONS_H
