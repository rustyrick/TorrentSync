#include <torrentsync/dht/NodeTree.h>

#include <exception>
#include <numeric>

namespace torrentsync
{
namespace dht
{

NodeTree::NodeTree(
    const NodeData nodeNode) : _node(nodeNode)
{
    clear();
}

NodeTree::~NodeTree()
{
}

bool NodeTree::addNode( NodeSPtr address )
{
    if (!address.get())
        throw std::invalid_argument("Node is not set");

    UpgradableLock rlock(mutex);

    BucketContainer::const_iterator bucket_it = findBucket(*address);
    assert(bucket_it != _buckets.end());

    BucketContainer::key_type bucket = *bucket_it;
    assert(bucket->inBounds(address));

    const bool isAdded = bucket->add(address);
    if ( !isAdded && bucket->inBounds(_node) )
    {
        UpgradedWriteLock wlock(rlock);
        MaybeBuckets maybe_split_buckets = split(bucket_it);

        // unsplittable, ignore it
        if (!maybe_split_buckets)
            return isAdded;

        BucketSPtrPair& split_buckets = *maybe_split_buckets;

        assert(split_buckets.first->inBounds(address) ||
               split_buckets.second->inBounds(address));

        if (split_buckets.first->inBounds(address))
        {
            split_buckets.first->add(address);
        }
        else
        {
            assert(split_buckets.second->inBounds(address));
            split_buckets.second->add(address);
        }
    }

    return isAdded;
}

void NodeTree::removeNode( NodeSPtr address )
{
    if (!address.get())
        throw std::invalid_argument("Node is not set");

    UpgradableLock rlock(mutex);

    BucketContainer::const_iterator bucket_it = findBucket(*address);
    assert(bucket_it != _buckets.end());

    BucketContainer::key_type bucket = *bucket_it;
    assert(bucket->inBounds(address));

    bucket->remove(*address);
}

size_t NodeTree::size() const noexcept
{
    ReadLock rlock(mutex);
    return std::accumulate(_buckets.begin(), _buckets.end(), static_cast<size_t>(0),
        [](const size_t init,const BucketContainer::key_type& t) -> size_t
            { return init+t->size(); });
}

BucketContainer::const_iterator
NodeTree::findBucket( const NodeData& addr ) const
{
    const BucketContainer::key_type key(new Bucket(addr,addr));

    // @TODO should not be linear search
    BucketContainer::const_iterator it = _buckets.begin();
    while ( it != _buckets.end() && ! (*it)->inBounds(addr))
    {
        ++it;
    }
    assert( it != _buckets.end() );
    assert( it->get() );
    return it;
}

MaybeBuckets
NodeTree::split( BucketContainer::const_iterator bucket_it )
{
    assert(bucket_it != _buckets.end());
    assert(bucket_it->get());
    BucketSPtr bucket = *bucket_it;

    MaybeBounds bounds = NodeData::splitInHalf(
            bucket->getLowerBound(), bucket->getUpperBound());

    if (!bounds)
        return MaybeBuckets();

    BucketSPtr lower_bucket(new Bucket(bucket->getLowerBound(),bounds->first));
    BucketSPtr upper_bucket(new Bucket(bounds->second,bucket->getUpperBound()));
    
    // split addresses in buckets
    for( Bucket::const_iterator it = bucket->cbegin(); it != bucket->cend(); ++it)
    {
        if (lower_bucket->inBounds(*it))
        {
            lower_bucket->add(*it);
        }
        else
        {
            assert(upper_bucket->inBounds(*it));
            upper_bucket->add(*it);
        }
    }

    assert(upper_bucket->size() + lower_bucket->size() == bucket->size());
 
    _buckets.insert(bucket_it,upper_bucket);
    _buckets.insert(bucket_it,lower_bucket);
    _buckets.erase(bucket_it);

    return MaybeBuckets(BucketSPtrPair(lower_bucket,upper_bucket));
}

void NodeTree::clear() noexcept
{
    WriteLock lock(mutex);
    _buckets.clear();

    // initialize first bucket
    boost::shared_ptr<Bucket> bucket(
            new Bucket( NodeData::minValue, NodeData::maxValue));
    _buckets.insert(bucket);
}

const boost::optional<NodeSPtr> NodeTree::getNode(
    const NodeData& data ) const
{
    return (*findBucket(data))->find(data); 
}

const NodeData& NodeTree::getTableNode() const noexcept
{
    return _node;
}

size_t NodeTree::getBucketsCount() const noexcept
{
    return _buckets.size();
}

}; // dht
}; // torrentsync
