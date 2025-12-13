#include <catch2/catch_all.hpp>
#include <QDebug>

TEST_CASE("FirstTest")
{
    INFO("Hello from Catch");
    REQUIRE(0 == 0);
}

TEST_CASE("SecondTest")
{
    INFO("Hello from Catch");
    REQUIRE(0 == 1);
}
