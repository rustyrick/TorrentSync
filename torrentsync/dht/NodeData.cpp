
#include <sstream>
#include <algorithm>
#include <cctype>
#include <stdexcept>

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/integer_traits.hpp>
#include <boost/integer.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/mersenne_twister.hpp>

#include <torrentsync/dht/NodeData.h>
#include <torrentsync/utils/RandomGenerator.h>
#include <torrentsync/utils/Buffer.h>
#include <torrentsync/utils/log/Logger.h>

namespace
{

template <size_t N>
class hexconverter
{
public:
    typedef typename boost::uint_t<N>::fast value_type;
    value_type value;
	value_type operator()() { return value; }
    friend std::istream& operator>>( std::istream& in, hexconverter<N>& out )
    {
        in >> std::hex >> out.value;
	return in;
    }
};

typedef hexconverter<32> uint32hex;
typedef hexconverter<64> uint64hex;
}; // anonymous namespace

namespace torrentsync
{
namespace dht
{

const uint32_t NodeData::ADDRESS_STRING_LENGTH = 40;
const size_t NodeData::addressDataLength = 20;

NodeData::NodeData(const torrentsync::utils::Buffer& buff)
{
    if ( buff.size() != addressDataLength) 
    {
        std::stringstream msg;
        msg << "NodeData expects " << addressDataLength << " byte as address";
        throw std::invalid_argument(msg.str());
    }
    
    const uint32_t * const data = reinterpret_cast<const uint32_t*>(buff.get());
    
    p1 = data[0];
    p1 <<= 32;
    p1 |= data[1];
    p2 =  data[2];
    p2 <<= 32;
    p2 |= data[3];
    p3 =  data[4];
}

NodeData::~NodeData()
{
}

void NodeData::parseString( const std::string& str )
{
    if (str.length() != 40 ||
            !std::all_of(str.begin(),str.end(), ::isxdigit))
    {
        std::string msg = "Invalid argument: "; msg += str;
        throw std::invalid_argument(msg);
    }

	p1 = boost::lexical_cast<uint64hex>(str.substr(0,16))();
	p2 = boost::lexical_cast<uint64hex>(str.substr(16,16))();
	p3 = boost::lexical_cast<uint32hex>(str.substr(32))();
}

const std::string NodeData::string() const
{
    std::string ret;
    ret.reserve(40);
    std::stringstream ss;
    ss << std::hex;
    ss.width(16); ss.fill('0'); ss << p1;
    ss.width(16); ss.fill('0'); ss << p2;
    ss.width(8);  ss.fill('0'); ss << p3;
    ss >> ret;
    return ret;
}

const NodeData NodeData::minValue =
    NodeData::parse("0000000000000000000000000000000000000000");
const NodeData NodeData::maxValue =
    NodeData::parse("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");

MaybeBounds NodeData::splitInHalf(
    const NodeData& low,
    const NodeData& high)
{
    NodeData half_low,half_high;

    // a masks 1..10....0 where the 1s are the root of the tree mask
    // low & mask == high & mask
    NodeData mask;
    mask.p1 = ~(low.p1 ^ high.p1);
    mask.p2 = ~(low.p2 ^ high.p2);
    mask.p3 = ~(low.p3 ^ high.p3);

    // bit mask full set, can't split anymore
    if (~mask.p3 == 0 && ~mask.p2 == 0 && ~mask.p3 == 0)
        return MaybeBounds();

    // create the mask to set/unset the new bit to add to next addresses
    NodeData new_bit = mask;
    const bool p2_first_bit = new_bit.p2 & 1;
    const bool p1_first_bit = new_bit.p1 & 1;
    new_bit.p1 >>= 1;
    new_bit.p1 |= 0x8000000000000000;
    new_bit.p2 >>= p1_first_bit;
    new_bit.p2 |= 0x8000000000000000 * (p1_first_bit); // set bit for carryover
    new_bit.p3 >>= p2_first_bit;
    new_bit.p3 |= 0x80000000 * (p2_first_bit); // set bit for carryover
    new_bit.p1 ^= mask.p1;
    new_bit.p2 ^= mask.p2;
    new_bit.p3 ^= mask.p3;

    // assert (only one bit set in all p1, p2, p3

    // creating half_low (the high of the lower pair)
    half_low = low;
    // also all the bits on the right of the bit are 0s, and must be 1s
    half_low.p1 |= ~(mask.p1 | new_bit.p1);
    half_low.p2 |= ~(mask.p2 | new_bit.p2);
    half_low.p3 |= ~(mask.p3 | new_bit.p3);

    // creating half_high (the low of the higher pair)
    half_high = high;
    // also all the bits on the right of the bit are 1s, and must be 0s
    half_high.p1 &= mask.p1 | new_bit.p1;
    half_high.p2 &= mask.p2 | new_bit.p2;
    half_high.p3 &= mask.p3 | new_bit.p3;

    return MaybeBounds(Bounds(half_low,half_high));
}

const NodeData NodeData::getRandom()
{
    NodeData data;
    using namespace torrentsync::utils;

    data.p1 = RandomGenerator::getInstance().get();
    data.p1 <<= 32;
    data.p1 |= RandomGenerator::getInstance().get();

    data.p2 = RandomGenerator::getInstance().get();
    data.p2 <<= 32;
    data.p2 |= RandomGenerator::getInstance().get();

    data.p3 = RandomGenerator::getInstance().get();

    return data;
}

void NodeData::read(
    torrentsync::utils::Buffer::const_iterator begin,
    const torrentsync::utils::Buffer::const_iterator end  )
{
    if ( end < begin || (static_cast<size_t>((end-begin)) < addressDataLength) )
    {
        LOG(ERROR,"NodeData - parseNode: not enough data to parse. Expected " << addressDataLength << ", found: " << (end-begin) );
        throw std::invalid_argument("Wrong amount of data to parse");
    }

    p1 = 0;
    p2 = 0;
    p3 = 0;

    for ( int i = 0; i <= 7; ++i )
    {
        p1 <<= 8;
        p1 |= *(begin++);
    }

    for ( int i = 8; i <= 15; ++i )
    {
        p2 <<= 8;
        p2 |= *(begin++);
    }

    for ( int i = 16; i <= 19; ++i )
    {
        p3 <<= 8;
        p3 |= *(begin++);
    }
}

torrentsync::utils::Buffer NodeData::write() const
{
    torrentsync::utils::Buffer buff(addressDataLength);
    
    buff.get()[0]  = p1 >> 56 & 0xFF; 
    buff.get()[1]  = p1 >> 48 & 0xFF;
    buff.get()[2]  = p1 >> 40 & 0xFF;
    buff.get()[3]  = p1 >> 32 & 0xFF;
    buff.get()[4]  = p1 >> 24 & 0xFF;
    buff.get()[5]  = p1 >> 16 & 0xFF;
    buff.get()[6]  = p1 >>  8 & 0xFF;
    buff.get()[7]  = p1 >>  0 & 0xFF;
    buff.get()[8]  = p2 >> 56 & 0xFF;
    buff.get()[9]  = p2 >> 48 & 0xFF;
    buff.get()[10] = p2 >> 40 & 0xFF;
    buff.get()[11] = p2 >> 32 & 0xFF;
    buff.get()[12] = p2 >> 24 & 0xFF;
    buff.get()[13] = p2 >> 16 & 0xFF;
    buff.get()[14] = p2 >>  8 & 0xFF;
    buff.get()[15] = p2 >>  0 & 0xFF;
    buff.get()[16] = p3 >> 24 & 0xFF;
    buff.get()[17] = p3 >> 16 & 0xFF;
    buff.get()[18] = p3 >>  8 & 0xFF;
    buff.get()[19] = p3 >>  0 & 0xFF;

    return buff;
}

}; // dht
}; // torrentsync