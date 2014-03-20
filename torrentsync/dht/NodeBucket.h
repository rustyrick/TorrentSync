#pragma once

#include <torrentsync/dht/NodeData.h>
#include <torrentsync/dht/Node.h>

#include <boost/shared_ptr.hpp>
#include <boost/array.hpp>
#include <boost/utility.hpp>
#include <boost/optional.hpp>

namespace torrentsync
{
namespace dht
{

template <size_t MaxSizeT>
class NodeBucket : public boost::noncopyable
{
public:
    NodeBucket(
            const NodeData& low,
            const NodeData& high );

    ~NodeBucket();

    typedef boost::array<boost::shared_ptr<Node>, MaxSizeT > NodeList;
    typedef typename boost::array<boost::shared_ptr<Node>, MaxSizeT >::const_iterator const_iterator;
    typedef typename boost::array<boost::shared_ptr<Node>, MaxSizeT >::iterator iterator;

    bool add(
        const boost::shared_ptr<Node> addr);

    bool remove(
        const Node& addr);

    const boost::optional<NodeSPtr> find(
        const NodeData& addr) const;

    void removeBad();

    inline size_t size()    const;
    inline size_t maxSize() const;

    void clear();

    bool inBounds(
            const boost::shared_ptr<Node> addr ) const;
    bool inBounds(
            const NodeData& addr ) const;

    inline const NodeData& getLowerBound() const { return low;  };
    inline const NodeData& getUpperBound() const { return high; };

    inline const_iterator begin()  const { return cbegin(); }
    inline const_iterator end()    const { return cend(); }

    inline const_iterator cbegin() const { return _elements.begin(); }
    inline const_iterator cend()   const { return _elements.begin()+addressCount; }

    std::ostream& string( std::ostream& out ) const;

private:
    
    inline iterator begin()       { return _elements.begin(); }
    inline iterator end()         { return _elements.begin()+addressCount; }

    NodeData low;
    NodeData high;

    size_t addressCount;

    mutable NodeList _elements;
};

template <size_t MaxSizeT>
NodeBucket<MaxSizeT>::NodeBucket(
            const NodeData& low,
            const NodeData& high ) :
    low(low), high(high),
    addressCount(0)
{
}

template <size_t MaxSizeT>
NodeBucket<MaxSizeT>::~NodeBucket()
{
    clear();
}

template <size_t MaxSizeT>
size_t NodeBucket<MaxSizeT>::size() const
{
    return addressCount;
}

template <size_t MaxSizeT>
size_t NodeBucket<MaxSizeT>::maxSize() const
{
    return MaxSizeT;
}

template <size_t MaxSizeT>
void NodeBucket<MaxSizeT>::clear()
{
    std::for_each( begin(), end(), 
            [](boost::shared_ptr<Node>& t) { assert(t.get()); t.reset();});
    addressCount = 0;
}

template <size_t MaxSizeT>
bool NodeBucket<MaxSizeT>::add( const boost::shared_ptr<Node> addr )
{
    if (!addr.get())
        throw std::invalid_argument("Node is NULL");

    if (*addr > high || *addr < low)
    {
        throw std::invalid_argument("The address can't stay in this Bucket: "
            + addr->string() + " "+low.string()+"-"+high.string());
    }

    if (size() == maxSize())
    {
        removeBad();
    }

    if (size() < maxSize())
    {
        // find first greater than the current address
        iterator it = std::find_if( begin(), end(),
            bind1st(std::less<boost::shared_ptr<Node> >(),addr) );
        std::copy_backward(it,end(),end()+1);
        *it = addr;
        ++addressCount;
        return true;
    }

    return false;
}

template <size_t MaxSizeT>
void NodeBucket<MaxSizeT>::removeBad()
{
    const iterator it = std::remove_if( begin(), end(),
            [](const boost::shared_ptr<Node>& addr) -> bool {
                assert(addr.get());
                return addr->isBad(); }
            );
    addressCount = it-begin();
}

template <size_t MaxSizeT>
bool NodeBucket<MaxSizeT>::inBounds(
        const boost::shared_ptr<Node> addr ) const
{
    if (!addr.get())
        throw std::invalid_argument("Node is NULL");
        
    return inBounds(*addr);
}

template <size_t MaxSizeT>
bool NodeBucket<MaxSizeT>::inBounds(
        const NodeData& addr ) const
{
    return low <= addr && addr <= high;
}

template <size_t MaxSizeT>
bool NodeBucket<MaxSizeT>::remove(
    const Node& addr)
{
    bool found = false;
    for ( size_t it = 0; it < addressCount; ++it)
    {
        const boost::shared_ptr<Node>& v = _elements[it];
        assert(v.get());

        if (addr != *v)
            continue;

        if (it+1 == addressCount) 
        {
            _elements[it] = boost::shared_ptr<Node>();
            --addressCount;
        }
        else
        {
            for( size_t in = it+1; in < addressCount; ++in )
            {
                _elements[it] = _elements[in];
            }
            --addressCount;
            _elements[addressCount] = boost::shared_ptr<Node>();
        }
        found = true;
    }

    return found;
}

template <size_t MaxSizeT>
std::ostream& operator<<( std::ostream& out, const NodeBucket<MaxSizeT>& element )
{
    return element.string(out);
}

template <size_t MaxSizeT>
std::ostream& NodeBucket<MaxSizeT>::string( std::ostream& out ) const
{
    out << low.string() << ' ' << high.string() << std::endl;
    for( const_iterator it = cbegin(); it != cend(); ++it )
    {
        assert(it->get());
        out << **it << std::endl;
    }
    return out;
}

template <size_t MaxSizeT>
const boost::optional<NodeSPtr> NodeBucket<MaxSizeT>::find(
    const NodeData& addr) const
{
    const_iterator it = std::find_if( begin(), end(), 
            [addr](const boost::shared_ptr<Node>& bucket_addr) -> bool {
                assert(bucket_addr.get());
                return static_cast<NodeData&>(*bucket_addr) == addr; }
            );

    boost::optional<NodeSPtr> ret;
    if ( it != end() )
    {
        ret = *it;
    }
    return ret;
}

}; // dht
}; // torrentsync

namespace std
{
template <size_t Size>
struct less<torrentsync::dht::NodeBucket<Size> >
{
      bool operator()
          (const torrentsync::dht::NodeBucket<Size>& x
          , const torrentsync::dht::NodeBucket<Size>& y) const
      {return x.getUpperBound() < y.getLowerBound();}
};
}; // std
