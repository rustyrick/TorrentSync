#include <boost/test/unit_test.hpp>

#include <torrentsync/dht/message/FindNode.h>
#include <torrentsync/dht/Node.h>
#include <torrentsync/utils/Yield.h>

#include <test/torrentsync/dht/CommonNodeTest.h>

#include <sstream>


BOOST_AUTO_TEST_SUITE(torrentsync_dht_message_FindNode);

using namespace torrentsync::dht::message;
using namespace torrentsync;

BOOST_AUTO_TEST_CASE(generation_1)
{
    utils::Buffer ret;

    std::string transaction = "aa";
    const char bytestring[20] = {'a','b','c','d','e','f','g','h','i','j','0','1','2','3','4','5','6','7','8','9'};
    const char bytestring_target[20] = {'m','n','o','p','q','r','s','t','u','v','w','x','y','z','1','2','3','4','5','6'};
    utils::Buffer b = putInBuffer(bytestring);
    utils::Buffer tb = putInBuffer(bytestring_target);
    dht::NodeData data, target;
    data.read(b.cbegin(),b.cend());
    target.read(tb.cbegin(),tb.cend());

    BOOST_REQUIRE_NO_THROW(
        ret = FindNode::getMessage(transaction,data,target)); 
    BOOST_REQUIRE_EQUAL(
        torrentsync::utils::Buffer("d1:ad2:id20:abcdefghij01234567896:target20:mnopqrstuvwxyz123456e1:q9:find_node1:t2:aa1:y1:qe"),ret);
}

BOOST_AUTO_TEST_CASE(parse)
{
    utils::Buffer b("d1:ad2:id20:abcdefghij01234567896:target20:mnopqrstuvwxyz123456e1:q9:find_node1:t2:aa1:y1:qe");

    auto m = torrentsync::dht::message::Message::parseMessage(b);
    BOOST_REQUIRE(!!m);
    BOOST_REQUIRE_EQUAL(m->getType(), Type::Query);
    BOOST_REQUIRE_EQUAL(m->getMessageType(), Messages::FindNode);

    auto p = dynamic_cast<dht::message::FindNode*>(m.get());
    BOOST_REQUIRE(p);

    BOOST_REQUIRE_EQUAL(p->getID(),"abcdefghij0123456789");
    BOOST_REQUIRE_EQUAL(p->getTransactionID(),"aa");
    BOOST_REQUIRE_NO_THROW(
            BOOST_REQUIRE_EQUAL(p->getTarget(),"mnopqrstuvwxyz123456"));
    BOOST_REQUIRE_THROW(p->getNodes(),
        MalformedMessageException);
}

BOOST_AUTO_TEST_CASE(reply_perfectMatch)
{
    const std::string id1("4747474747474747474747474747474747474747");
    const std::string id2("4848484848484848484848484848484848484848");
    
    const utils::Buffer transaction("aa");
    
    utils::Buffer buff("d1:rd2:id20:GGGGGGGGGGGGGGGGGGGG5:nodes26:HHHHHHHHHHHHHHHHHHHHEEEEGGe1:t2:aa1:y1:re");
    
    dht::NodeData source(dht::NodeData::parse(id1));
    dht::NodeData target(dht::NodeData::parse(id2));
    
    // 0x4747 => 'GG', 0x45454545 => 'EEEE'
    boost::optional<boost::asio::ip::udp::endpoint> endpoint(
        boost::asio::ip::udp::endpoint(
             boost::asio::ip::address_v4(0x45454545),0x4747));
    
    boost::shared_ptr<dht::Node> match(new dht::Node(target.write(),endpoint));
    
    std::list<dht::NodeSPtr> nodes;
    nodes.push_back(match);

    utils::Buffer ret;
    BOOST_REQUIRE_NO_THROW(
        ret = FindNode::getMessageReply(
            transaction,
            source,
            utils::makeYield(nodes.cbegin(),nodes.cend()).function()));
            
    BOOST_REQUIRE_EQUAL(ret,buff);
}

BOOST_AUTO_TEST_CASE(reply_multiple)
{
    const std::string id1("4747474747474747474747474747474747474747");
    const std::string id2("4848484848484848484848484848484848484848");
    
    const utils::Buffer transaction("aa");
    
    utils::Buffer buff("d1:rd2:id20:GGGGGGGGGGGGGGGGGGGG5:nodes78:HHHHHHHHHHHHHHHHHHHHEEEEGGHHHHHHHHHHHHHHHHHHHHEEEEGGHHHHHHHHHHHHHHHHHHHHEEEEGGe1:t2:aa1:y1:re");
    
    dht::NodeData source(dht::NodeData::parse(id1));
    dht::NodeData target(dht::NodeData::parse(id2));
    
    // 0x4747 => 'GG', 0x45454545 => 'EEEE'
    boost::optional<boost::asio::ip::udp::endpoint> endpoint(
        boost::asio::ip::udp::endpoint(
             boost::asio::ip::address_v4(0x45454545),0x4747));
    
    boost::shared_ptr<dht::Node> match(new dht::Node(target.write(),endpoint));
    
    std::list<dht::NodeSPtr> nodes;
    nodes.push_back(match);
    nodes.push_back(match);
    nodes.push_back(match);

    utils::Buffer ret;
    BOOST_REQUIRE_NO_THROW(
        ret = FindNode::getMessageReply(
            transaction,
            source,
            utils::makeYield(nodes.cbegin(),nodes.cend()).function()));
            
    BOOST_REQUIRE_EQUAL(ret,buff);
}

BOOST_AUTO_TEST_SUITE_END();
