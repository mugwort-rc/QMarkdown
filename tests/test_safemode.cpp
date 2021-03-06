#include "test_safemode.h"

#include <QDebug>
#include <QTest>

TestSafeMode::TestSafeMode()
{

}

TestSafeMode::~TestSafeMode()
{

}

void TestSafeMode::initTestCase()
{

}

void TestSafeMode::cleanupTestCase()
{

}

void TestSafeMode::init()
{
    this->escape_md = markdown::create_Markdown(markdown::Markdown::escape_mode);
    this->remove_md = markdown::create_Markdown(markdown::Markdown::remove_mode);
    this->replace_md = markdown::create_Markdown(markdown::Markdown::replace_mode);
}

void TestSafeMode::cleanup()
{

}

void TestSafeMode::html_then_blockquote()
{
    QString converted = this->escape_md->convert(
                "to:" "\n"
                "" "\n"
                R"(<td /><td style="text-align: center; white-space: nowrap;"><br />)" "\n"
                "" "\n"
                "> 3) You don't need to alter all localization files." "\n"
                ">    Adding the new labels to the en_US files will do it." "\n"
                "");
    QCOMPARE(converted,
             QString("<p>to:</p>" "\n"
                     R"(<p>&lt;td /&gt;&lt;td style="text-align: center; white-space: nowrap;"&gt;&lt;br /&gt;</p>)" "\n"
                     "<blockquote>" "\n"
                     "<p>3) You don't need to alter all localization files." "\n"
                     "   Adding the new labels to the en_US files will do it.</p>" "\n"
                     "</blockquote>"));
}

void TestSafeMode::inline_html_advanced()
{
    QString converted = this->escape_md->convert(
                "Simple block on one line:" "\n"
                "" "\n"
                "<div>foo</div>" "\n"
                "" "\n"
                "And nested without indentation:" "\n"
                "" "\n"
                "<div>" "\n"
                "<div>" "\n"
                "<div>" "\n"
                "foo" "\n"
                "</div>" "\n"
                "</div>" "\n"
                "<div>bar</div>" "\n"
                "</div>" "\n"
                "");
    QCOMPARE(converted,
             QString("<p>Simple block on one line:</p>" "\n"
                     "<p>&lt;div&gt;foo&lt;/div&gt;</p>" "\n"
                     "<p>And nested without indentation:</p>" "\n"
                     "<p>&lt;div&gt;" "\n"
                     "&lt;div&gt;" "\n"
                     "&lt;div&gt;" "\n"
                     "foo" "\n"
                     "&lt;/div&gt;" "\n"
                     "&lt;/div&gt;" "\n"
                     "&lt;div&gt;bar&lt;/div&gt;" "\n"
                     "&lt;/div&gt;</p>"));
}

void TestSafeMode::inline_html_comments()
{
    QString converted = this->escape_md->convert(
                "Paragraph one." "\n"
                "" "\n"
                "<!-- This is a simple comment -->" "\n"
                "" "\n"
                "<!--" "\n"
                "\tThis is another comment." "\n"
                "-->" "\n"
                "" "\n"
                "Paragraph two." "\n"
                "" "\n"
                "<!-- one comment block -- -- with two comments -->" "\n"
                "" "\n"
                "The end." "\n"
                "");
    QCOMPARE(converted,
             QString("<p>Paragraph one.</p>" "\n"
                     "<p>&lt;!-- This is a simple comment --&gt;</p>" "\n"
                     "<p>&lt;!--" "\n"
                     "    This is another comment." "\n"
                     "--&gt;</p>" "\n"
                     "<p>Paragraph two.</p>" "\n"
                     "<p>&lt;!-- one comment block -- -- with two comments --&gt;</p>" "\n"
                     "<p>The end.</p>"));
}

void TestSafeMode::inline_html_simple()
{
    QString converted = this->escape_md->convert(
                "Here's a simple block:" "\n"
                "" "\n"
                "<div>" "\n"
                "\tfoo" "\n"
                "</div>" "\n"
                "" "\n"
                "This should be a code block, though:" "\n"
                "" "\n"
                "\t<div>" "\n"
                "\t\tfoo" "\n"
                "\t</div>" "\n"
                "" "\n"
                "As should this:" "\n"
                "" "\n"
                "\t<div>foo</div>" "\n"
                "" "\n"
                "Now, nested:" "\n"
                "" "\n"
                "<div>" "\n"
                "\t<div>" "\n"
                "\t\t<div>" "\n"
                "\t\t\tfoo" "\n"
                "\t\t</div>" "\n"
                "\t</div>" "\n"
                "</div>" "\n"
                "" "\n"
                "This should just be an HTML comment:" "\n"
                "" "\n"
                "<!-- Comment -->" "\n"
                "" "\n"
                "Multiline:" "\n"
                "" "\n"
                "<!--" "\n"
                "Blah" "\n"
                "Blah" "\n"
                "-->" "\n"
                "" "\n"
                "Code block:" "\n"
                "" "\n"
                "\t<!-- Comment -->" "\n"
                "" "\n"
                "Just plain comment, with trailing spaces on the line:" "\n"
                "" "\n"
                "<!-- foo -->   " "\n"
                "" "\n"
                "Code:" "\n"
                "" "\n"
                "\t<hr />" "\n"
                "\t" "\n"
                "Hr's:" "\n"
                "" "\n"
                "<hr>" "\n"
                "" "\n"
                "<hr/>" "\n"
                "" "\n"
                "<hr />" "\n"
                "" "\n"
                "<hr>   " "\n"
                "" "\n"
                "<hr/>  " "\n"
                "" "\n"
                "<hr /> " "\n"
                "" "\n"
                R"(<hr class="foo" id="bar" />)" "\n"
                "" "\n"
                R"(<hr class="foo" id="bar"/>)" "\n"
                "" "\n"
                R"(<hr class="foo" id="bar" >)" "\n"
                "" "\n"
                "<some [weird](http://example.com) stuff>" "\n"
                "");
    QCOMPARE(converted,
             QString("<p>Here's a simple block:</p>" "\n"
                     "<p>&lt;div&gt;" "\n"
                     "    foo" "\n"
                     "&lt;/div&gt;</p>" "\n"
                     "<p>This should be a code block, though:</p>" "\n"
                     "<pre><code>&lt;div&gt;" "\n"
                     "    foo" "\n"
                     "&lt;/div&gt;" "\n"
                     "</code></pre>" "\n"
                     "<p>As should this:</p>" "\n"
                     "<pre><code>&lt;div&gt;foo&lt;/div&gt;" "\n"
                     "</code></pre>" "\n"
                     "<p>Now, nested:</p>" "\n"
                     "<p>&lt;div&gt;" "\n"
                     "    &lt;div&gt;" "\n"
                     "        &lt;div&gt;" "\n"
                     "            foo" "\n"
                     "        &lt;/div&gt;" "\n"
                     "    &lt;/div&gt;" "\n"
                     "&lt;/div&gt;</p>" "\n"
                     "<p>This should just be an HTML comment:</p>" "\n"
                     "<p>&lt;!-- Comment --&gt;</p>" "\n"
                     "<p>Multiline:</p>" "\n"
                     "<p>&lt;!--" "\n"
                     "Blah" "\n"
                     "Blah" "\n"
                     "--&gt;</p>" "\n"
                     "<p>Code block:</p>" "\n"
                     "<pre><code>&lt;!-- Comment --&gt;" "\n"
                     "</code></pre>" "\n"
                     "<p>Just plain comment, with trailing spaces on the line:</p>" "\n"
                     "<p>&lt;!-- foo --&gt;   </p>" "\n"
                     "<p>Code:</p>" "\n"
                     "<pre><code>&lt;hr /&gt;" "\n"
                     "</code></pre>" "\n"
                     "<p>Hr's:</p>" "\n"
                     "<p>&lt;hr&gt;</p>" "\n"
                     "<p>&lt;hr/&gt;</p>" "\n"
                     "<p>&lt;hr /&gt;</p>" "\n"
                     "<p>&lt;hr&gt;   </p>" "\n"
                     "<p>&lt;hr/&gt;  </p>" "\n"
                     "<p>&lt;hr /&gt; </p>" "\n"
                     R"(<p>&lt;hr class="foo" id="bar" /&gt;</p>)" "\n"
                     R"(<p>&lt;hr class="foo" id="bar"/&gt;</p>)" "\n"
                     R"(<p>&lt;hr class="foo" id="bar" &gt;</p>)" "\n"
                     R"(<p>&lt;some <a href="http://example.com">weird</a> stuff&gt;</p>)"));
}

void TestSafeMode::link_targets()
{
    QString converted = this->escape_md->convert(
                "[XSS](javascript://%0Aalert%28'XSS'%29;)" "\n"
                "See http://security.stackexchange.com/q/30330/1261 for details." "\n"
                "" "\n"
                "");
    QCOMPARE(converted,
             QString(R"(<p><a href="">XSS</a>)" "\n"
                     "See http://security.stackexchange.com/q/30330/1261 for details.</p>"));
}

void TestSafeMode::remove()
{
    QString converted = this->remove_md->convert(
                "Here's a simple block:" "\n"
                "" "\n"
                "<div>" "\n"
                "\tfoo" "\n"
                "</div>" "\n"
                "" "\n"
                "This should be a code block, though:" "\n"
                "" "\n"
                "\t<div>" "\n"
                "\t\tfoo" "\n"
                "\t</div>" "\n"
                "" "\n"
                "As should this:" "\n"
                "" "\n"
                "\t<div>foo</div>" "\n"
                "" "\n"
                "Now, nested:" "\n"
                "" "\n"
                "<div>" "\n"
                "\t<div>" "\n"
                "\t\t<div>" "\n"
                "\t\t\tfoo" "\n"
                "\t\t</div>" "\n"
                "\t</div>" "\n"
                "</div>" "\n"
                "" "\n"
                "This should just be an HTML comment:" "\n"
                "" "\n"
                "<!-- Comment -->" "\n"
                "" "\n"
                "Multiline:" "\n"
                "" "\n"
                "<!--" "\n"
                "Blah" "\n"
                "Blah" "\n"
                "-->" "\n"
                "" "\n"
                "Code block:" "\n"
                "" "\n"
                "\t<!-- Comment -->" "\n"
                "" "\n"
                "Just plain comment, with trailing spaces on the line:" "\n"
                "" "\n"
                "<!-- foo -->   " "\n"
                "" "\n"
                "Code:" "\n"
                "" "\n"
                "\t<hr />" "\n"
                "\t" "\n"
                "Hr's:" "\n"
                "" "\n"
                "<hr>" "\n"
                "" "\n"
                "<hr/>" "\n"
                "" "\n"
                "<hr />" "\n"
                "" "\n"
                "<hr>   " "\n"
                "" "\n"
                "<hr/>  " "\n"
                "" "\n"
                "<hr /> " "\n"
                "" "\n"
                R"(<hr class="foo" id="bar" />)" "\n"
                "" "\n"
                R"(<hr class="foo" id="bar"/>)" "\n"
                "" "\n"
                R"(<hr class="foo" id="bar" >)" "\n"
                "" "\n"
                "<some [weird](http://example.com) stuff>" "\n"
                "");
    QCOMPARE(converted,
             QString("<p>Here's a simple block:</p>" "\n"
                     "<p></p>" "\n"
                     "<p>This should be a code block, though:</p>" "\n"
                     "<pre><code>&lt;div&gt;" "\n"
                     "    foo" "\n"
                     "&lt;/div&gt;" "\n"
                     "</code></pre>" "\n"
                     "<p>As should this:</p>" "\n"
                     "<pre><code>&lt;div&gt;foo&lt;/div&gt;" "\n"
                     "</code></pre>" "\n"
                     "<p>Now, nested:</p>" "\n"
                     "<p></p>" "\n"
                     "<p>This should just be an HTML comment:</p>" "\n"
                     "<p></p>" "\n"
                     "<p>Multiline:</p>" "\n"
                     "<p></p>" "\n"
                     "<p>Code block:</p>" "\n"
                     "<pre><code>&lt;!-- Comment --&gt;" "\n"
                     "</code></pre>" "\n"
                     "<p>Just plain comment, with trailing spaces on the line:</p>" "\n"
                     "<p></p>" "\n"
                     "<p>Code:</p>" "\n"
                     "<pre><code>&lt;hr /&gt;" "\n"
                     "</code></pre>" "\n"
                     "<p>Hr's:</p>" "\n"
                     "<p></p>" "\n"
                     "<p></p>" "\n"
                     "<p></p>" "\n"
                     "<p></p>" "\n"
                     "<p></p>" "\n"
                     "<p></p>" "\n"
                     "<p></p>" "\n"
                     "<p></p>" "\n"
                     "<p></p>" "\n"
                     "<p></p>"));
}

void TestSafeMode::replace()
{
    QString converted = this->replace_md->convert(
                "Here's a simple block:" "\n"
                "" "\n"
                "<div>" "\n"
                "\tfoo" "\n"
                "</div>" "\n"
                "" "\n"
                "This should be a code block, though:" "\n"
                "" "\n"
                "\t<div>" "\n"
                "\t\tfoo" "\n"
                "\t</div>" "\n"
                "" "\n"
                "As should this:" "\n"
                "" "\n"
                "\t<div>foo</div>" "\n"
                "" "\n"
                "Now, nested:" "\n"
                "" "\n"
                "<div>" "\n"
                "\t<div>" "\n"
                "\t\t<div>" "\n"
                "\t\t\tfoo" "\n"
                "\t\t</div>" "\n"
                "\t</div>" "\n"
                "</div>" "\n"
                "" "\n"
                "This should just be an HTML comment:" "\n"
                "" "\n"
                "<!-- Comment -->" "\n"
                "" "\n"
                "Multiline:" "\n"
                "" "\n"
                "<!--" "\n"
                "Blah" "\n"
                "Blah" "\n"
                "-->" "\n"
                "" "\n"
                "Code block:" "\n"
                "" "\n"
                "\t<!-- Comment -->" "\n"
                "" "\n"
                "Just plain comment, with trailing spaces on the line:" "\n"
                "" "\n"
                "<!-- foo -->   " "\n"
                "" "\n"
                "Code:" "\n"
                "" "\n"
                "\t<hr />" "\n"
                "\t" "\n"
                "Hr's:" "\n"
                "" "\n"
                "<hr>" "\n"
                "" "\n"
                "<hr/>" "\n"
                "" "\n"
                "<hr />" "\n"
                "" "\n"
                "<hr>   " "\n"
                "" "\n"
                "<hr/>  " "\n"
                "" "\n"
                "<hr /> " "\n"
                "" "\n"
                R"(<hr class="foo" id="bar" />)" "\n"
                "" "\n"
                R"(<hr class="foo" id="bar"/>)" "\n"
                "" "\n"
                R"(<hr class="foo" id="bar" >)" "\n"
                "" "\n"
                "<some [weird](http://example.com) stuff>" "\n"
                "");
    QCOMPARE(converted,
             QString("<p>Here's a simple block:</p>" "\n"
                     "<p>[HTML_REMOVED]</p>" "\n"
                     "<p>This should be a code block, though:</p>" "\n"
                     "<pre><code>&lt;div&gt;" "\n"
                     "    foo" "\n"
                     "&lt;/div&gt;" "\n"
                     "</code></pre>" "\n"
                     "<p>As should this:</p>" "\n"
                     "<pre><code>&lt;div&gt;foo&lt;/div&gt;" "\n"
                     "</code></pre>" "\n"
                     "<p>Now, nested:</p>" "\n"
                     "<p>[HTML_REMOVED]</p>" "\n"
                     "<p>This should just be an HTML comment:</p>" "\n"
                     "<p>[HTML_REMOVED]</p>" "\n"
                     "<p>Multiline:</p>" "\n"
                     "<p>[HTML_REMOVED]</p>" "\n"
                     "<p>Code block:</p>" "\n"
                     "<pre><code>&lt;!-- Comment --&gt;" "\n"
                     "</code></pre>" "\n"
                     "<p>Just plain comment, with trailing spaces on the line:</p>" "\n"
                     "<p>[HTML_REMOVED]</p>" "\n"
                     "<p>Code:</p>" "\n"
                     "<pre><code>&lt;hr /&gt;" "\n"
                     "</code></pre>" "\n"
                     "<p>Hr's:</p>" "\n"
                     "<p>[HTML_REMOVED]</p>" "\n"
                     "<p>[HTML_REMOVED]</p>" "\n"
                     "<p>[HTML_REMOVED]</p>" "\n"
                     "<p>[HTML_REMOVED]</p>" "\n"
                     "<p>[HTML_REMOVED]</p>" "\n"
                     "<p>[HTML_REMOVED]</p>" "\n"
                     "<p>[HTML_REMOVED]</p>" "\n"
                     "<p>[HTML_REMOVED]</p>" "\n"
                     "<p>[HTML_REMOVED]</p>" "\n"
                     "<p>[HTML_REMOVED]</p>"));
}

void TestSafeMode::script_tags()
{
    QString converted = this->escape_md->convert(
                "This should be stripped/escaped in safe_mode." "\n"
                "" "\n"
                "<script>" "\n"
                R"(alert("Hello world!"))" "\n"
                "</script>" "\n"
                "" "\n"
                "With blank lines." "\n"
                "" "\n"
                "<script>" "\n"
                "" "\n"
                R"(alert("Hello world!"))" "\n"
                "" "\n"
                "</script>" "\n"
                "" "\n"
                "Now with some weirdness" "\n"
                "" "\n"
                "``<script <!--" "\n"
                R"(alert("Hello world!"))" "\n"
                "</script <>`` `" "\n"
                "" "\n"
                "Try another way." "\n"
                "" "\n"
                "<script <!--" "\n"
                R"(alert("Hello world!"))" "\n"
                "</script <>" "\n"
                "" "\n"
                "This time with blank lines." "\n"
                "" "\n"
                "<script <!--" "\n"
                "" "\n"
                R"(alert("Hello world!"))" "\n"
                "" "\n"
                "</script <>" "\n"
                "");
    QCOMPARE(converted,
             QString("<p>This should be stripped/escaped in safe_mode.</p>" "\n"
                     "<p>&lt;script&gt;" "\n"
                     R"(alert("Hello world!"))" "\n"
                     "&lt;/script&gt;</p>" "\n"
                     "<p>With blank lines.</p>" "\n"
                     "<p>&lt;script&gt;</p>" "\n"
                     R"(<p>alert("Hello world!")</p>)" "\n"
                     "<p>&lt;/script&gt;</p>" "\n"
                     "<p>Now with some weirdness</p>" "\n"
                     "<p><code>&lt;script &lt;!--" "\n"
                     R"(alert("Hello world!"))" "\n"
                     "&lt;/script &lt;&gt;</code> `</p>" "\n"
                     "<p>Try another way.</p>" "\n"
                     "<p>&lt;script &lt;!--" "\n"
                     R"(alert("Hello world!"))" "\n"
                     "&lt;/script &lt;&gt;</p>" "\n"
                     "<p>This time with blank lines.</p>" "\n"
                     "<p>&lt;script &lt;!--</p>" "\n"
                     R"(<p>alert("Hello world!")</p>)" "\n"
                     "<p>&lt;/script &lt;&gt;</p>"));
}

void TestSafeMode::unsafe_urls()
{
    QString converted = this->escape_md->convert(
                "These links should be unsafe and not allowed in safe_mode" "\n"
                "" "\n"
                "[link](javascript:alert%28'Hello%20world!'%29)" "\n"
                "[link](vbscript:msgbox%28%22Hello%20world!%22%29)" "\n"
                "[link](livescript:alert%28'Hello%20world!'%29)" "\n"
                "[link](mocha:[code])" "\n"
                "[link](jAvAsCrIpT:alert%28'Hello%20world!'%29)" "\n"
                "[link](ja&#32;vas&#32;cr&#32;ipt:alert%28'Hello%20world!'%29)" "\n"
                "[link](ja&#00032;vas&#32;cr&#32;ipt:alert%28'Hello%20world!'%29)" "\n"
                "[link](ja&#x00020;vas&#32;cr&#32;ipt:alert%28'Hello%20world!'%29)" "\n"
                "[link](ja%09&#x20;%0Avas&#32;cr&#x0a;ipt:alert%28'Hello%20world!'%29)" "\n"
                "[link](ja%20vas%20cr%20ipt:alert%28'Hello%20world!'%29)" "\n"
                "[link](live%20script:alert%28'Hello%20world!'%29)" "\n"
                "" "\n"
                "![img](javascript:alert%29'XSS'%29)" "\n"
                "[ref][]" "\n"
                "![imgref][]" "\n"
                "" "\n"
                "[ref]: javascript:alert%29'XSS'%29" "\n"
                "[imgref]: javascript:alert%29'XSS'%29" "\n"
                "" "\n"
                "These should work regardless:" "\n"
                "" "\n"
                "[relative](relative/url.html)" "\n"
                "[email](mailto:foo@bar.com)" "\n"
                "[news scheme](news:some.news.group.com)" "\n"
                "[http link](http://example.com)" "\n"
                "");
    QCOMPARE(converted,
             QString("<p>These links should be unsafe and not allowed in safe_mode</p>" "\n"
                     R"(<p><a href="">link</a>)" "\n"
                     R"(<a href="">link</a>)" "\n"
                     R"(<a href="">link</a>)" "\n"
                     R"(<a href="">link</a>)" "\n"
                     R"(<a href="">link</a>)" "\n"
                     R"(<a href="">link</a>)" "\n"
                     R"(<a href="">link</a>)" "\n"
                     R"(<a href="">link</a>)" "\n"
                     R"(<a href="">link</a>)" "\n"
                     R"(<a href="">link</a>)" "\n"
                     R"(<a href="">link</a></p>)" "\n"
                     R"(<p><img alt="img" src="" />)" "\n"
                     R"(<a href="">ref</a>)" "\n"
                     R"(<img alt="imgref" src="" /></p>)" "\n"
                     "<p>These should work regardless:</p>" "\n"
                     R"(<p><a href="relative/url.html">relative</a>)" "\n"
                     R"(<a href="mailto:foo@bar.com">email</a>)" "\n"
                     R"(<a href="news:some.news.group.com">news scheme</a>)" "\n"
                     R"(<a href="http://example.com">http link</a></p>)"));
}
