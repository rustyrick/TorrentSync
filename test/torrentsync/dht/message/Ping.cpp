#include <boost/test/unit_test.hpp>

#include <torrentsync/dht/message/Ping.h>
#include <torrentsync/dht/NodeData.h>

#include <test/torrentsync/dht/CommonNodeTest.h>

#include <sstream>


BOOST_AUTO_TEST_SUITE(torrentsync_dht_message_Ping);

using namespace torrentsync::dht::message;
using namespace torrentsync;

BOOST_AUTO_TEST_CASE(generation_1)
{
    utils::Buffer ret;

    auto transaction = utils::makeBuffer("aa");
    utils::Buffer b = {71,71,71,71,71,71,71,71,72,72,72,72,72,72,72,72,73,73,73,73};
    dht::NodeData data;
    data.read(b.cbegin(),b.cend());

    BOOST_REQUIRE_NO_THROW(
        ret = Ping::getQuery(transaction,data)); 
    BOOST_REQUIRE(
        utils::makeBuffer("d1:ad2:id20:GGGGGGGGHHHHHHHHIIIIe1:q4:ping1:t2:aa1:y1:qe") == ret);
}

BOOST_AUTO_TEST_CASE(parse)
{
    auto b = utils::makeBuffer("d1:ad2:id20:GGGGGGGGHHHHHHHHIIIIe1:q4:ping1:t2:aa1:y1:qe");

    auto m = dht::message::Message::parseMessage(b);
    BOOST_REQUIRE(!!m);
    BOOST_REQUIRE(m->getType() == Type::Query);
    BOOST_REQUIRE(*m->getMessageType() == Messages::Ping);

    auto p = dynamic_cast<dht::message::Ping*>(m.get());
    BOOST_REQUIRE(p);

    BOOST_REQUIRE(p->getID() == "GGGGGGGGHHHHHHHHIIII");
    BOOST_REQUIRE(p->getTransactionID() == "aa");
}

BOOST_AUTO_TEST_CASE(parseBinary)
{
    auto b = utils::makeBuffer("d1:ad2:id20:GGGGGGGGHHHHHHHHIIIIe1:q4:ping1:t2:aa1:y1:qe");
    b[15] = '\t';
    b[18] = '\0';

    auto m = dht::message::Message::parseMessage(b);
    BOOST_REQUIRE(!!m);
    BOOST_REQUIRE(m->getType() == Type::Query);
    BOOST_REQUIRE(*m->getMessageType() == Messages::Ping);

    auto p = dynamic_cast<dht::message::Ping*>(m.get());
    BOOST_REQUIRE(p);

    auto id = utils::makeBuffer("GGGGGGGGHHHHHHHHIIII");
    id[3] = '\t';
    id[6] = '\0';

    BOOST_REQUIRE(p->getID() == id);
    BOOST_REQUIRE(p->getTransactionID() == "aa");
}

BOOST_AUTO_TEST_CASE(parseRandom)
{
    for ( size_t i = 0; i < TEST_LOOP_COUNT; ++i )
    {
        //! set address to 0s and then copy something inside
        auto b = utils::makeBuffer("d1:ad2:id20:00000000000000000000e1:q4:ping1:t2:aa1:y1:qe");

        auto addr = dht::NodeData::getRandom();
        auto ab = addr.write();
        std::copy(ab.cbegin(),ab.cend(),b.begin()+12);

        auto m = dht::message::Message::parseMessage(b);
        BOOST_REQUIRE(!!m);
        BOOST_REQUIRE(m->getType() == Type::Query);
        BOOST_REQUIRE(*m->getMessageType() == Messages::Ping);

        auto p = dynamic_cast<dht::message::Ping*>(m.get());
        BOOST_REQUIRE(p);

        BOOST_REQUIRE(p->getID() == ab);
        BOOST_REQUIRE(p->getTransactionID() == "aa");
    }
}

BOOST_AUTO_TEST_SUITE_END()
