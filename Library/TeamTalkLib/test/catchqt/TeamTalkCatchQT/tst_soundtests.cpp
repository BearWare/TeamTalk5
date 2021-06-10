#include <catch2/catch.hpp>
#include <QDebug>

TEST_CASE("FirstTest")
{
    INFO("Hello from Catch");
    qInfo() << "Hello from QT debug";
    REQUIRE(0 == 1);
}
