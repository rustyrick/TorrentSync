#pragma once

#include <string>

#include <boost/shared_ptr.hpp>
#include <torrentsync/dht/message/BEncodeDecoder.h>

namespace torrentsync
{
namespace dht
{
namespace message
{

namespace Type
{
    extern const std::string Query;
    extern const std::string Response;
    extern const std::string Error;
};

namespace Field
{
    extern const std::string TransactionID;
    extern const std::string Type;
    extern const std::string QueryType;
    extern const std::string ResponseType;
    extern const std::string ErrorType;
    extern const std::string Arguments;

    extern const std::string PeerID;
};

namespace Messages
{
    extern const std::string Ping;
};

class MalformedMessageException : public std::runtime_error
{
public:
    MalformedMessageException( const std::string& what ) : std::runtime_error(what) {}
};

//! Abstract class representing every message
class Message : protected torrentsync::dht::message::BEncodeDecoder
{
public:
    virtual ~Message() {}

    //! Parse a generic message and returns an instance of it.
    //! This method must be used to parse messages.
    //! @param istream the input stream to read from
    //! @return a shared pointer with the message
    //! @throw BEncodeDecoderException in case an error is encountered while
    //! parsing
    static boost::shared_ptr<Message> parseMessage( std::istream& istream );

    //! Returns the message type. In this way you can cast to the correct
    //! object.
    //! @return a member of the Messages namespace
    //! @throw MalformedMessageException in case the field is not available (it's mandatory)
    virtual const std::string getMessageType() const;

    //! returns the type of the message
    //! @return a member of Type namespace
    //! @throw MalformedMessageException in case the field is not available (it's mandatory)
    virtual const std::string getType() const;

protected:
    inline Message() {}

};

} /* message */
} /* dht */
} /* torrentsync */
