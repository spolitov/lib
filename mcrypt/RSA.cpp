#include "pch.h"

#include "RSA.h"

using namespace std;
using namespace boost;

namespace mcrypt {

//////////////////////////////////////////////////////////////////////////
// class BNHolder
//////////////////////////////////////////////////////////////////////////

class BNTraits {
public:
    static BIGNUM * null() { return NULL; }
    static void close(BIGNUM * num) { BN_free(num); }
};

typedef mstd::handle_base<BIGNUM*, mstd::comparable_traits<BNTraits> > BNHolder;

//////////////////////////////////////////////////////////////////////////
// class RSA::Holder
//////////////////////////////////////////////////////////////////////////

class RSATraits {
public:
    static ::RSA* null() { return NULL; }
    static void close(::RSA * rsa) { RSA_free(rsa); }
};

typedef mstd::handle_base< ::RSA*, mstd::comparable_traits<RSATraits> > HolderBase;

class RSA::Holder : public HolderBase {
public:
    Holder(::RSA * rsa) 
        : HolderBase(rsa) {}
};

//////////////////////////////////////////////////////////////////////////
// class RSA
//////////////////////////////////////////////////////////////////////////

static void handleError()
{
    error_t err = ERR_get_error();
    if(err)
        throw RSAException(err);
}

static void checkError(int result)
{
    if(result == -1)
        handleError();
}

static void invokeListener(int a, int b, void * raw)
{
    RSA::GenerateListener * listener = static_cast<RSA::GenerateListener*>(raw);
    (*listener)(a, b);
}

static BIGNUM * extractBignum(std::vector<char>::const_iterator & i, std::vector<char>::const_iterator end)
{
    if(end - i < 4)
        throw RSAException(0);
    int len = ntohl(*mstd::pointer_cast<const int*>(&*i));
    i += 4;
    if(end - i < len)
        throw RSAException(0);
    BIGNUM * result = BN_bin2bn(mstd::pointer_cast<const unsigned char*>(&*i), len, NULL);
    i += len;
    return result;
}

RSAPtr RSA::generateKey(int num, unsigned long e, const mcrypt::RSA::GenerateListener & listener)
{
    return RSAPtr(new RSA(new RSA::Holder(RSA_generate_key(num, e, invokeListener, const_cast<void*>(static_cast<const void*>(&listener))))));
}

RSAPtr RSA::createFromPublicKey(const std::vector<char> & src)
{
    std::vector<char>::const_iterator i = src.begin();
    BNHolder n(extractBignum(i, src.end()));
    BNHolder e(extractBignum(i, src.end()));
    
    scoped_ptr<RSA::Holder> holder(new RSA::Holder(RSA_new()));
    if(!holder->handle())
        handleError();
    holder->handle()->n = n.release();
    holder->handle()->e = e.release();
    
    return RSAPtr(new RSA(holder));
}

RSAPtr RSA::createFromPrivateKey(const std::vector<char> & src)
{
    std::vector<char>::const_iterator i = src.begin();
    BNHolder n(extractBignum(i, src.end()));
    BNHolder e(extractBignum(i, src.end()));
    BNHolder d(extractBignum(i, src.end()));
    BNHolder p(extractBignum(i, src.end()));
    BNHolder q(extractBignum(i, src.end()));
    BNHolder dmp1(extractBignum(i, src.end()));
    BNHolder dmq1(extractBignum(i, src.end()));
    BNHolder iqmp(extractBignum(i, src.end()));

    scoped_ptr<RSA::Holder> holder(new RSA::Holder(RSA_new()));
    if(!holder->handle())
        handleError();
    holder->handle()->n = n.release();
    holder->handle()->e = e.release();
    holder->handle()->d = d.release();
    holder->handle()->p = p.release();
    holder->handle()->q = q.release();
    holder->handle()->dmp1 = dmp1.release();
    holder->handle()->dmq1 = dmq1.release();
    holder->handle()->iqmp = iqmp.release();
        
    int code = RSA_check_key(holder->handle());
    
    checkError(code);
    
    if(code == 0)
        throw RSAException(0);
    
    return RSAPtr(new RSA(holder));
}

RSA::RSA(RSA::Holder * holder)
    : holder_(holder)
{
    if(!holder->handle())
        handleError();
}

RSA::RSA(scoped_ptr<RSA::Holder> & holder)
{
    holder_.swap(holder);

    if(!holder_->handle())
        handleError();
}

RSA::~RSA()
{
}

const RSA::Holder & RSA::holder() const
{
    return *holder_;
}

int RSA::size() const
{
    return RSA_size(holder().handle());
}

typedef vector<BIGNUM*> BigNums;

std::vector<char> packBigNums(const BigNums & nums)
{
    size_t size = 0;
    for(BigNums::const_iterator i = nums.begin(); i != nums.end(); ++i)
        size += 4 + BN_num_bytes(*i);
    
    std::vector<char> result(size);
    std::vector<char>::iterator p = result.begin();
    for(BigNums::const_iterator i = nums.begin(); i != nums.end(); ++i)
    {
        BIGNUM * num = *i;
        size_t len = BN_num_bytes(num);
        *mstd::pointer_cast<boost::uint32_t*>(&*p) = htonl(static_cast<boost::uint32_t>(len));
        p += 4;
        BN_bn2bin(num, mstd::pointer_cast<unsigned char*>(&*p));
        p += len;
    }

    return result;
}

int getPadding(Padding padding, bool publicEncrypt)
{
    switch(padding) {
    case pdNone:
        return RSA_NO_PADDING;
    case pdDefault:
        return publicEncrypt ? RSA_PKCS1_OAEP_PADDING : RSA_PKCS1_PADDING;
    case pdPKCS1:
        return RSA_PKCS1_PADDING;
    case pdPKCS1_OAEP:
        return RSA_PKCS1_OAEP_PADDING;
    }
    BOOST_ASSERT(false);
    return RSA_NO_PADDING;
}

template<class Func>
static std::vector<char> process(Func func, const char * src, size_t len, ::RSA * rsa, int padding)
{
    std::vector<char> result(RSA_size(rsa));
    int sz = func(static_cast<int>(len), mstd::pointer_cast<const unsigned char*>(src), mstd::pointer_cast<unsigned char*>(&result[0]), rsa, padding);
    checkError(sz);
    result.resize(sz);
    return result;
}

std::vector<char> RSA::publicEncrypt(const char * src, size_t len, Padding padding) const
{
    return process(&RSA_public_encrypt, src, len, holder().handle(), getPadding(padding, true));
}

std::vector<char> RSA::publicDecrypt(const char * src, size_t len,  Padding padding) const
{
    return process(&RSA_public_decrypt, src, len, holder().handle(), getPadding(padding, false));
}

std::vector<char> RSA::privateEncrypt(const char * src, size_t len, Padding padding) const
{
    return process(&RSA_private_encrypt, src, len, holder().handle(), getPadding(padding, false));
}

std::vector<char> RSA::privateDecrypt(const char * src, size_t len, Padding padding) const
{
    return process(&RSA_private_decrypt, src, len, holder().handle(), getPadding(padding, true));
}

size_t getPaddingTail(int padding)
{
    switch(padding) {
    case RSA_NO_PADDING:
        return 0;
    case RSA_PKCS1_PADDING:
        return RSA_PKCS1_PADDING_SIZE;
    case RSA_PKCS1_OAEP_PADDING:
        return 41;
    default:
        BOOST_ASSERT(false);
        return 0;
    };
};

template<class Func>
static std::vector<char> processEx(Func func, const char * src, size_t len, ::RSA * rsa, bool encrypt, int padding)
{
    if(!len)
        return std::vector<char>();
    size_t keySize = RSA_size(rsa);
    size_t tailSize = getPaddingTail(padding);
    size_t inputBlockSize;
    size_t outputBlockSize;
    if(encrypt)
    {
        inputBlockSize = keySize - tailSize;
        outputBlockSize = keySize;
    } else {
        inputBlockSize = keySize;
        outputBlockSize = keySize - tailSize;
    }
    const char * end = src + len;
    std::vector<char> result((len + inputBlockSize - 1) / inputBlockSize * outputBlockSize);
    char * out = &result[0];
    while(src != end)
    {
        size_t clen = (src + inputBlockSize <= end ? inputBlockSize : end - src);
        int sz = func(static_cast<int>(clen), mstd::pointer_cast<const unsigned char*>(src), mstd::pointer_cast<unsigned char*>(&result[0]), rsa, padding);
        checkError(sz);
        out += sz;
        src += clen;
    }
    result.resize(out - &result[0]);
    return result;
}

std::vector<char> RSA::publicEncryptEx(const char * src, size_t len, Padding padding) const
{
    return processEx(&RSA_public_encrypt, src, len, holder().handle(), true, getPadding(padding, true));
}

std::vector<char> RSA::publicDecryptEx(const char * src, size_t len,  Padding padding) const
{
    return processEx(&RSA_public_decrypt, src, len, holder().handle(), false, getPadding(padding, false));
}

std::vector<char> RSA::privateEncryptEx(const char * src, size_t len, Padding padding) const
{
    return processEx(&RSA_private_encrypt, src, len, holder().handle(), true, getPadding(padding, false));
}

std::vector<char> RSA::privateDecryptEx(const char * src, size_t len, Padding padding) const
{
    return processEx(&RSA_private_decrypt, src, len, holder().handle(), false, getPadding(padding, true));
}

std::vector<char> RSA::extractPublicKey() const
{
    ::RSA * src = holder().handle();
    
    BigNums nums;
    nums.push_back(src->n);
    nums.push_back(src->e);
    
    return packBigNums(nums);
}

std::vector<char> RSA::extractPrivateKey() const
{
    ::RSA * src = holder().handle();
    
    BigNums nums;
    nums.push_back(src->n);
    nums.push_back(src->e);
    nums.push_back(src->d);
    nums.push_back(src->p);
    nums.push_back(src->q);
    nums.push_back(src->dmp1);
    nums.push_back(src->dmq1);
    nums.push_back(src->iqmp);
    
    return packBigNums(nums);
}

//////////////////////////////////////////////////////////////////////////
// class RSAException
//////////////////////////////////////////////////////////////////////////

std::string getErrorMessage(error_t error)
{
    char buffer[0x1000];
    ERR_load_crypto_strings();
    
    ERR_error_string_n(error, buffer, sizeof(buffer));
    return buffer;
}

RSAException::RSAException(error_t error)
    : what_(getErrorMessage(error)), error_(error) {}
    
const char * RSAException::what() const throw ()
{
    return what_.c_str();
}

error_t RSAException::error() const
{
    return error_;
}

}
