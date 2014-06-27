#include <torrentsync/utils/log/Logger.h>
#include <torrentsync/dht/RoutingTable.h>
#include <torrentsync/dht/message/Ping.h>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>

#include <vector>

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/assign/list_of.hpp>

//! list of known bootstrap servers for the DHT network
static const std::list<
    boost::tuple<std::string,unsigned short> > BOOTSTRAP_ADDRESSES =
        boost::assign::tuple_list_of
            ("router.bittorrent.com",6881)
            ("router.utorrent.com"  ,6881);

//! 3 batches per second while initializing the DHT, should be configurable.
static const size_t INITIALIZE_PING_BATCH_INTERVAL = static_cast<size_t>(1000/3);

//! 5 addresses per batch while initialing the DHT, should be configurable.
static const size_t INITIALIZE_PING_BATCH_SIZE     = 5;

//! The minimum number of nodes in the table under which we'll try to
//! connect at the bootstrap addresses
static const size_t MINIMUM_NODES_UNTIL_BOOTSTRAP = 10;

namespace torrentsync
{
namespace dht
{

RoutingTable::RoutingTable(
    const udp::endpoint& endpoint)
    : _table(NodeData::getRandom()),
      _socket(io_service)
{
    LOG(INFO, "RoutingTable * Peer Node: " << _table.getPeerNode());
    LOG(INFO, "RoutingTable * Bind Node: " << endpoint);
    initializeNetwork(endpoint);
}

RoutingTable::~RoutingTable()
{
}

boost::asio::io_service& RoutingTable::getIO_service()
{
    return io_service;
}

udp::endpoint RoutingTable::getEndpoint() const
{
    return _socket.local_endpoint();
}

void RoutingTable::initializeTable( shared_timer timer )
{
    if (_initial_addresses.empty())
    {
        LOG(INFO, "RoutingTable: table initialization from previously known addresses terminated");

        const size_t table_size = _table.size();
        if ( table_size < MINIMUM_NODES_UNTIL_BOOTSTRAP )
        {
            LOG(INFO,"Proceeding with boostrap procedure from known nodes: "
                    << table_size << ','
                    << MINIMUM_NODES_UNTIL_BOOTSTRAP);

            //! TODO proceed with bootstrap
            throw std::runtime_error("Not Implemented Yet");
        }
        return;
    }

    if (!timer)
    {
        timer = shared_timer(new boost::asio::deadline_timer(io_service,
            boost::posix_time::milliseconds(INITIALIZE_PING_BATCH_INTERVAL)));
    }

    LOG(DEBUG, "RoutingTable: Register initializeTable timer");
    timer->async_wait(
        [this,timer] (const boost::system::error_code& e) {
                if ( e.value() != 0 )
                    LOG(ERROR, "Error in RoutingTable initializeTable timer: " << e.message());
                using namespace boost::asio;

                assert(!_initial_addresses.empty());

                const ip::address ip = _initial_addresses.front();
                _initial_addresses.pop_front(); // copy local and delete head

                LOG(DEBUG, "RoutingTable: initializing ping with " << ip);

                //! create and send the message
                torrentsync::utils::Buffer msg =
                    torrentsync::dht::message::Ping::getMessage(
                        torrentsync::utils::Buffer("aab"),
                        _table.getPeerNode());

                //! send the message

                //! TODO send find_node to get our node
                // give the 8 nodes priority over others. once we start having
                // enough fresh nodes we don't need additional old nodes and we
                // can dump them.
                
                LOG(ERROR, "RoutingTable::initializeTable lambda not implemented yet!");

                LOG(DEBUG, "RoutingTable: " << _initial_addresses.size() <<
                                " initializing addresses remaining");

                if ( ! _initial_addresses.empty() )
                    initializeTable(timer);
            });
}

void RoutingTable::tableMaintenance()
{
    throw std::runtime_error("Not Implemented Yet");
}

void RoutingTable::initializeNetwork(
    const udp::endpoint& endpoint )
{
    // throws boost::system::system_error
    _socket.open(endpoint.protocol());
    _socket.bind(endpoint);
}

void RoutingTable::registerCallback(
    const Callback::callback& func,
    const std::string& type,
    const std::string& messageType,
    const torrentsync::dht::NodeData& source, 
    const boost::optional<torrentsync::utils::Buffer>& transactionID)
{
    _callbacks.insert(
        std::make_pair(
            source,
            Callback(func,type,messageType,source,transactionID)));
    throw std::runtime_error("Not Implemented Yet");
}

boost::optional<Callback> RoutingTable::getCallback(
    const message::Message& message)
{
    auto its = _callbacks.equal_range(message.getID());

    boost::optional<Callback> ret;
    for( auto it = its.first; it != its.second; ++it )
    {
        if (it->second.verifyConstraints(message))
        {
            ret = it->second;
            _callbacks.erase(it);
            break;
        }
    }

    return ret;
}

void RoutingTable::sendMessage()
{
    throw std::runtime_error("Not Implemented Yet");
}

void RoutingTable::recvMessage()
{
    throw std::runtime_error("Not Implemented Yet");
}

boost::shared_ptr<boost::asio::ip::tcp::socket> RoutingTable::lookForNode()
{
    throw std::runtime_error("Not Implemented Yet");
    boost::shared_ptr<boost::asio::ip::tcp::socket> ret;
    return ret;
}

}; // dht
}; // torrentsync

