#ifndef SHA1_H__
#define SHA1_H__

#include <cstdint>
#include <cstring>

/**
 Applies SHA-1 algorithm as descrtibed in:
 https://tools.ietf.org/html/rfc3174
 */




class Sha1
{
public:
    Sha1();
    ~Sha1();
    
    /**
    Can be used to cancel the processing of current message and bring this
    object to its initial state. Not needed before using a newly constructed object.    
    */
    void reset();

    /**
    Process the next part of the message string.
    Cannot be called before reset.
    Cannot be called feedExtraBits or padding or getResult until reset is called.
    */
    void feed(const char *str, size_t len);

    /**
    If message length in bits is not divisible by 8, the extra bits should be feeded with 
    this method.

    After this method is called, feed cannot be called until reset or padding is called.
    */
    void feedExtraBits(const char *str, int extraBits);

    /** 
    Adds the padding after all the message has been fed.
    Must be called before getResult.
    */
    void padding();

    /** 
    Used for retrieving output digest. Must be called after padding is called. Does not 
    reset object's state.    
    */
    void getResult(char* result);

    /**
    Processes a complete message. 
    Brings the object to its initial state after processing.

    len: 
        if extraBits = 0: total number of byte in the message.
        if extraBits > 0: total number of byte in the message excluding 
            the byte which contains the extra bits.
    extraBits: must be in [1, 7] range inclusive.
    result: A buffer with capacity of 20 bytes for outputting the digest. 
    */
    void processStrFull(const char *str, uint64_t len, int extraBits, char *result);

    /**
    Processes a single block. Used internally.

    M: An 64 byte buffer containing the block data to be processed.
    H(in/out): A five 32-bit word buffer. Must contain the values of 
    digest accumulators prior to the processing of the supplied block. 
    This method writes updated values as output to H.
    */
    static void processBlockInternal(const unsigned char *M, uint32_t *H);

private:
    // Accumulators for digest:
    uint32_t H[5];

    // Used for counting the total number of bits in the message. Used in padding.
    uint64_t numBits;

    // Used for keeping a partial block while input is being fed.
    unsigned char partial[64]; 

    // Number of bytes in partial block.
    int partialLen;

    // Number of extra bits supplied with method feedExtraBits.
    int extraBits;

    // Private copy ctor and copy assignment operator.
    Sha1(const Sha1&) { }
    Sha1 operator=(const Sha1&) { return *this; }

};


#endif // include guard

