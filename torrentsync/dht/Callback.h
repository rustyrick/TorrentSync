#pragma once

#include <torrentsync/utils/Buffer.h>
#include <torrentsync/dht/Peer.h>

#include <functional>

#include <boost/optional.hpp>

namespace torrentsync
{
namespace dht
{

namespace message
{
class Message;
}


/*  Callback constains a function to call when a message has arrived matching
 *  specific constraints.  In this way we can easily write asynchronous code on
 *  the server side to process either single query-reply DHT messages, and also
 *  more complex sessions.
 *
 *  This class is kind of generic, in the future it may be modified and moved
 *  to a more generic place in case it's needed by other parts of the
 *  applications.
 *
 * */

class Callback
{
public:
    //! after 3 minutes a callback is condisered old
    static const size_t TIME_LIMIT;

    //! type of the callback
    typedef std::function<bool (const torrentsync::dht::message::Message&)> callback;

    //! type of the peer filter 
    typedef boost::optional<torrentsync::dht::Peer> filterPeer;

    //! type of the trasaction filter 
    typedef boost::optional<torrentsync::utils::Buffer> filterTransactionID;

    //! Constructor
    //! @param function         Callback do call
    //! @param source           Filter by the source address
    //! @param transactionID    Filter by transaction ID
    Callback(
        const callback& function,
        const std::string& type,
        const filterPeer& source,
        const filterTransactionID& transactionID);

    //! Calls the callback function
    const bool call( const torrentsync::dht::message::Message& m ) const  { return _func(m); }

    //! verifies if the callback is too old.
    //! @returns true if the callback is more than TIME_LIMIT seconds old.
    bool isOld() const;

    //! verifies if the callback respects the filters set
    bool verifyConstraints( const torrentsync::dht::message::Message& message ) const;

private:

    //! callback function to call
    callback _func;

    //! filter condition for string type (mandatory)
    std::string _type;

    //! filter condition for source address
    boost::optional<torrentsync::dht::Peer> _source;

    //! filter condition for transaction it
    boost::optional<torrentsync::utils::Buffer> _transactionID;
    
    //! creation time, to filter out old callbacks
    time_t _creation_time;

};

}; // dht
}; // torrentsync

