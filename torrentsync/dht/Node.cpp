#include <torrentsync/dht/Node.h>
#include <torrentsync/utils/log/Logger.h>

#include <boost/integer_traits.hpp>

namespace torrentsync
{
namespace dht
{

const time_t Node::good_interval              = 15 * 60;  // 15 minutes
const size_t Node::allowed_unanswered_queries = 10;

Node::Node()
{
    setGood();
}

Node::Node( const Node& addr ) : Node()
{
    *this = addr;
    setGood();
}

Node::Node(
    const torrentsync::utils::Buffer& data,
    const boost::optional<udp::endpoint>& endpoint ) :
      NodeData(data), _endpoint(endpoint)
{
    setGood();
}

Node::Node(
    utils::Buffer::const_iterator begin,
    utils::Buffer::const_iterator end) : Node()
{
    read(begin,end);
}

void Node::setGood() noexcept
{
    _last_time_good = time(0);
    _last_unanswered_queries = 0;
}

bool Node::isGood() const noexcept
{
    return _last_time_good > time(0)-good_interval;
}

bool Node::isQuestionable() const noexcept
{
    return !isGood() && _last_unanswered_queries <= allowed_unanswered_queries;
}

bool Node::isBad() const noexcept
{
    return !isGood() && _last_unanswered_queries >  allowed_unanswered_queries;
}

const time_t& Node::getLastTimeGood() const noexcept
{
    return _last_time_good;
}

const boost::optional<udp::endpoint>& Node::getEndpoint() const noexcept
{
    return _endpoint;
}

void Node::setEndpoint( udp::endpoint& endpoint )
{
    _endpoint = endpoint;
}

void Node::read(
    torrentsync::utils::Buffer::const_iterator begin,
    torrentsync::utils::Buffer::const_iterator end)
{
    NodeData::read(begin,end);
    begin += NodeData::addressDataLength;

    if ( end > begin && static_cast<size_t>(end-begin) < PEERDATALENGTH)
    {
        LOG(ERROR,"Peer - parsePeer: not enough data to parse. Expected " << PEERDATALENGTH << ", found: " << (end-begin) );
        throw std::invalid_argument("Not enough data to parse Peer contact information");
    }

    uint32_t address;
    uint16_t port;
    
    for( size_t _i = 0; _i < sizeof(uint32_t); ++_i )
    {
        address <<= 8;
        address += *begin++;
    }
    
    const boost::asio::ip::address_v4 new_address(
        ntohl(address));
    
    port = *begin++;
    port <<= 8;
    port += *begin++;

    _endpoint = udp::endpoint(new_address,ntohs(port));
}

utils::Buffer Node::getPackedNode() const
{
    assert(!!_endpoint);
    
    utils::Buffer buff = NodeData::write();
    buff.reserve(PACKED_NODE_SIZE);

    auto networkOrderAddress        = htonl(_endpoint->address().to_v4().to_ulong());
    const uint16_t portNetworkOrder = htons(_endpoint->port());

    buff.push_back(networkOrderAddress >> 24);
    buff.push_back(networkOrderAddress >> 16);
    buff.push_back(networkOrderAddress >> 8);
    buff.push_back(networkOrderAddress);

    buff.push_back(portNetworkOrder >> 8);
    buff.push_back(portNetworkOrder);

    return buff;
}

}; // dht
}; // torrentsync

