#include <torrentsync/utils/log/Logger.h>
#include <torrentsync/utils/Buffer.h>
#include <torrentsync/dht/RoutingTable.h>
#include <torrentsync/dht/message/query/Ping.h>
#include <torrentsync/dht/message/query/FindNode.h>
#include <torrentsync/dht/message/reply/Ping.h>
#include <torrentsync/dht/message/reply/FindNode.h>
#include <torrentsync/dht/Callback.h>
#include <torrentsync/utils/Yield.h>

#include <exception> // for not implemented stuff

namespace torrentsync
{
namespace dht
{

using namespace torrentsync;
namespace msg = dht::message;

void RoutingTable::handlePingQuery(
    const dht::message::query::Ping& ping,
    const dht::Node&          node)
{
    assert(!!(node.getEndpoint()));
    
    // send ping reply
    sendMessage( msg::reply::Ping::make(
                    ping.getTransactionID(), _table.getTableNode()),
                 *(node.getEndpoint()) );
}

//! Handle ping reply.
void RoutingTable::handlePingReply(
    const dht::message::reply::Ping& message,
    const dht::Node& node)
{
    LOG(DEBUG,"Ping Reply received " << message.getID() << " " << node);
}

void RoutingTable::handleFindNodeQuery(
    const dht::message::query::FindNode& message,
    const dht::Node& node)
{
    LOG(DEBUG,"Find Query received " << message.getID() << " " << node);
    assert(!!(node.getEndpoint()));

    auto nodes = _table.getClosestNodes(node);
    sendMessage(
        msg::reply::FindNode::make(
            message.getTransactionID(),
            _table.getTableNode(),
            utils::makeYield<dht::NodeSPtr>(nodes.cbegin(),nodes.cend()).function()),
        *(node.getEndpoint()));
}

void RoutingTable::handleFindNodeReply(
    const dht::message::reply::FindNode& message,
    const dht::Node& node)
{
    LOG(WARN,"find_node reply without a callback. " << message.getID() << "-" << node );
}

void RoutingTable::doPing(
    dht::Node& destination )
{
    assert(!!(destination.getEndpoint()));

    torrentsync::utils::Buffer transaction = newTransaction();

    torrentsync::utils::Buffer ping = msg::query::Ping::make(
        transaction,
        _table.getTableNode());

    registerCallback([&](
            boost::optional<Callback::payload_type> data,
            const torrentsync::dht::Callback&       trigger) {

            if (!!data)
            {
                data->node.setGood(); // mark the node as good
                LOG(DEBUG,"Ping handled: " << data->node );
            }
        }, transaction, destination);

    // send ping reply
    sendMessage( ping, *(destination.getEndpoint()) );
}

} // dht
} // torrentsync
