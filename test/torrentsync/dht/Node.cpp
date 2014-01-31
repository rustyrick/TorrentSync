#include <boost/test/unit_test.hpp>

#include <torrentsync/dht/Node.h>
#include <test/torrentsync/dht/CommonAddressTest.h>

BOOST_AUTO_TEST_SUITE(torrentsync_dht_Node);

using namespace torrentsync::dht;

BOOST_AUTO_TEST_CASE(constructor)
{
    for ( int i = 0; i < TEST_LOOP_COUNT; ++i )
    {
        BOOST_REQUIRE_NO_THROW(
            udp::endpoint endpoint;
            Node n( Address(generateRandomAddress()), endpoint);
            BOOST_REQUIRE_EQUAL(endpoint,n.getEndpoint());
                );
    }
}

BOOST_AUTO_TEST_SUITE_END();

