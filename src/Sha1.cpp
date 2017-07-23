#include "Sha1.h"

#include <cstring>
#include <cassert>

#include <vector>
static void printBlock(const std::vector<unsigned char>& block);



/*
Print the value of a varible/expression along with it's name. 
Useful for quick debugging.
*/
#include <iostream>
#define PRINT(a) do { \
    std::cout << #a << "=" << a << std::endl; \
} while(0)




// Note: The algorithm only calls this function with n = 1, 5 and 30. 
// Calling with n = 0 would result in X >> 32 which is undefined.
inline static uint32_t S(int n, uint32_t X)
{
    return (X << n) | (X >> (32 - n));
}

inline static uint32_t f0(uint32_t B, uint32_t C, uint32_t D)
{
    return (B & C) | ((~B) & D);
}

inline static uint32_t f1(uint32_t B, uint32_t C, uint32_t D)
{
    return B ^ C ^ D;
}

inline static uint32_t f2(uint32_t B, uint32_t C, uint32_t D)
{
    return (B & C) | (B & D) | (C & D);
}

inline static uint32_t f3(uint32_t B, uint32_t C, uint32_t D)
{
    return B ^ C ^ D;
}

/**
Combine 4 consecutive bytes starting from src into a 32 bit 
unsigned integer in big-endian fashion.
*/
inline static uint32_t makeUint32(const unsigned char *src)
{
    uint32_t r = (*src++) << 24;
    r |= (*src++) << 16;
    r |= (*src++) << 8;
    r |= *src;
    return r;
}





Sha1::Sha1()
{ 
    reset();
}

Sha1::~Sha1()
{
}

void Sha1::reset()
{
    numBits = 0L;

    H[0] = 0x67452301;
    H[1] = 0xEFCDAB89;
    H[2] = 0x98BADCFE;
    H[3] = 0x10325476;
    H[4] = 0xC3D2E1F0;

    partialLen = 0;
    extraBits = 0;
}

void Sha1::getResult(char* result)
{
    int ri = 0;
    for(int i = 0; i < 5; i++) {
        int h = H[i];
        result[ri++] = h >> 24;
        result[ri++] = h >> 16;
        result[ri++] = h >> 8;
        result[ri++] = h;
    }
}

void Sha1::processStrFull(const char *str, uint64_t len, int extraBits, char *result)
{
    assert(extraBits >= 0 && extraBits < 8);

    reset();

    feed(str, len);
    
    if (extraBits > 0) {
        feedExtraBits(&str[len], extraBits);
    }

    padding();
    
    getResult(result);
}

void Sha1::feed(const char *str, size_t len)
{

    while(len > 0) {
        partial[partialLen] = *str++;
        numBits += 8L;
        ++partialLen;
        if (64 == partialLen) {
            processBlockInternal(partial, H);
            partialLen = 0;
        }
        len--;
    }

}

void Sha1::feedExtraBits(const char *str, int extraBits)
{
    assert(extraBits > 0 && extraBits < 8);

    this->extraBits = extraBits;

    // Copying the whole byte. We will have to set zero to unwanted 
    // bits later, while applying padding.
    partial[partialLen] = *str;
    
    numBits += extraBits;
}

void Sha1::padding()
{    
    int partialBits = partialLen * 8 + extraBits;

    int currentByte = partialBits / 8;
    int shift = 7 - (partialBits % 8);

    // Append a single bit of 1.
    uint32_t mask = 1 << shift;
    partial[currentByte] |= mask;

    // Extend the single bit in the mask to its left, 
    // i.e 00001000 --> 11111000
    mask |= mask << 1; 
    mask |= mask << 2;
    mask |= mask << 4;

    // Clear the extra bits that may have been copied by memcpy.
    partial[currentByte] &= mask;

    // Will padding require another block?
    if (partialBits > 512 - 64 - 1) {
        // Zero out the region after currentByte.

        // Note: when currentByte=63, &partial[currentByte+1] is an invalid pointer.
        if (currentByte < 63) {
            memset(&partial[currentByte + 1], 0, 64 - (currentByte + 1));
        }
        processBlockInternal(partial, H);
        //partialBits = 0;

        memset(partial, 0, 64 - 8);
        //partialBits = (64 - 8) * 8;
    } else {          
        // Zero out the region between currentByte and last 8 bytes.
        memset(&partial[currentByte + 1], 0, 64 - 8 - (currentByte + 1));
        //partialBits = (64 - 8) * 8;
    }

    // Write message length into the last 8 bytes.
    partial[56] = numBits >> 56;
    partial[57] = numBits >> 48;
    partial[58] = numBits >> 40;
    partial[59] = numBits >> 32;
    partial[60] = numBits >> 24;
    partial[61] = numBits >> 16;
    partial[62] = numBits >> 8;
    partial[63] = numBits;
    //partialBits = 512;

    processBlockInternal(partial, H);
    partialBits = 0;
}


void Sha1::processBlockInternal(const unsigned char *M, uint32_t *H)
{

    static const uint32_t K[4] = {
        0x5A827999,
        0x6ED9EBA1,
        0x8F1BBCDC,
        0xCA62C1D6
    };

    uint32_t W[80];

    for(int i = 0; i < 16; i++) {
        W[i] = makeUint32(M);

        // We won't use M anywhere else so we can modify it here.
        M += 4; 
    }

    for(int t = 16; t < 80; t++) {
        W[t] = S(1, W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16]);
    }

    uint32_t A, B, C, D, E;

    A = H[0]; 
    B = H[1]; 
    C = H[2]; 
    D = H[3]; 
    E = H[4];

    for(int t = 0; t < 20; t++) {
        uint32_t TEMP = S(5, A) + E + W[t] + K[0] + f0(B, C, D); E = D; D = C; C = S(30, B);  B = A; A = TEMP;
    }
    
    for(int t = 20; t < 40; t++) {
        uint32_t TEMP = S(5, A) + E + W[t] + K[1] + f1(B, C, D); E = D; D = C; C = S(30, B);  B = A; A = TEMP;
    }

    for(int t = 40; t < 60; t++) {
        uint32_t TEMP = S(5, A) + E + W[t] + K[2] + f2(B, C, D); E = D; D = C; C = S(30, B);  B = A; A = TEMP;
    }

    for(int t = 60; t < 80; t++) {
        uint32_t TEMP = S(5, A) + E + W[t] + K[3] + f3(B, C, D); E = D; D = C; C = S(30, B);  B = A; A = TEMP;
    }

    H[0] += A; 
    H[1] += B; 
    H[2] += C; 
    H[3] += D; 
    H[4] += E; 
}





