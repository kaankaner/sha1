#include <gtest/gtest.h>

#include "Sha1.h"










namespace {

/**
Useful for inspecting a block for debugging.
*/
void printBlock(const std::vector<unsigned char>& block)
{
    assert(64 == block.size());

    std::stringstream ss;
    ss << std::hex;

    for(int i = 0; i < 64; i++) {
        ss << (int)block[i] << " ";
        if (3 == i % 4) {
            ss << '\n';
        }
    }

    std::cout << ss.str();
}

const std::vector<std::string> testExpecteds = {
    "A9 99 3E 36 47 06 81 6A BA 3E 25 71 78 50 C2 6C 9C D0 D8 9D",
    "84 98 3E 44 1C 3B D2 6E BA AE 4A A1 F9 51 29 E5 E5 46 70 F1",
    "34 AA 97 3C D4 C4 DA A4 F6 1E EB 2B DB AD 27 31 65 34 01 6F",
    "DE A3 56 A2 CD DD 90 C7 A7 EC ED C5 EB B5 63 93 4F 46 04 52"
};

std::vector<std::string> constructTestInputs()
{
    // Test cases taken from rfc3174.
    const std::string TEST1("abc");
    const std::string TEST2a("abcdbcdecdefdefgefghfghighijhi");
    const std::string TEST2b("jkijkljklmklmnlmnomnopnopq");
    const std::string TEST2(TEST2a + TEST2b);
    const std::string TEST3("a");
    const std::string TEST4a("01234567012345670123456701234567");
    const std::string TEST4b("01234567012345670123456701234567");
    // an exact multiple of 512 bits
    const std::string TEST4 = std::string(TEST4a + TEST4b);    

    const std::string seeds[4] = {TEST1, TEST2, TEST3, TEST4};
    long int repeatcount[4] = {1, 1, 1000000, 10};

    std::vector<std::string> result;
    for(int i = 0; i < 4; i++) {
        std::stringstream ss;
        for(int j = 0; j < repeatcount[i]; j++) {
            ss << seeds[i];
        }
        result.push_back( ss.str() );
    }   

    return result;
}

/**
 Convert a given int [0, 16) to its hexadecimal digit version [0-9][A-F].
 */
char hexDigitToString(int x)
{
    assert(x >= 0 && x < 16);
    static const std::string table("0123456789ABCDEF");
    return table[x];
}


/**
Reads the first 20 characters at src. 
Prints them to a string in the same format with the test cases.
Returns the result.
*/
std::string convertToTestFormat(char *src)
{
    std::stringstream ss;
    for(int i = 0; i < 20; i++) {
        unsigned char c = static_cast<unsigned char>(src[i]);
        if (i > 0) ss << " ";
        ss << hexDigitToString(c / 16) << hexDigitToString(c % 16);

    }

    return ss.str();
}

} // end unnamed namespace

TEST(sha1, test1)
{
    std::vector<std::string> testInputs = constructTestInputs();

    for(int i = 0; i < (signed int)testInputs.size(); i++) {
        const std::string& input = testInputs[i];
        const std::string& expected = testExpecteds[i];
        char result[20];
        sha1::Sha1 sha1;
        sha1.processStrFull(input.c_str(), input.length(), 0, result);
        std::string result_str = convertToTestFormat(result);
        EXPECT_STREQ( expected.c_str(), result_str.c_str() );
    }   
}






namespace {

// Test cases involving the processing of a single block.
// Url: http://csrc.nist.gov/groups/ST/toolkit/documents/Examples/SHA1.pdf

struct BlockTestCase {

    // Block data.
    uint32_t W[16];

    // State of the accumulators before processing the block.
    uint32_t Hin[5];

    // State of the accumulators expected after processing the block.
    uint32_t Hout[5];
};


static BlockTestCase blockTestCases[] = {

    //First block for message "abc".
    {
        {   
            0x61626380, 0x00000000, 0x00000000, 0x00000000, 
            0x00000000, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0x00000000, 0x00000018
        },
        {0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0},
        {0xA9993E36, 0x4706816A, 0xBA3E2571, 0x7850C26C, 0x9CD0D89D}
    },

    // For message "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq":

    // First block:
    {
        {
            0x61626364, 0x62636465, 0x63646566, 0x64656667,
            0x65666768, 0x66676869, 0x6768696A, 0x68696A6B, 
            0x696A6B6C, 0x6A6B6C6D, 0x6B6C6D6E, 0x6C6D6E6F,
            0x6D6E6F70, 0x6E6F7071, 0x80000000, 0x00000000
        },
        {0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0},
        {0xF4286818, 0xC37B27AE, 0x0408F581, 0x84677148, 0x4A566572}
    },

    // Second block:
    {
        {
            0x00000000, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0x00000000, 0x000001C0,
        },
        {0xF4286818, 0xC37B27AE, 0x0408F581, 0x84677148, 0x4A566572},
        {0x84983E44, 0x1C3BD26E, 0xBAAE4AA1, 0xF95129E5, 0xE54670F1}
    }
};

/**
    Converts an array of 16 uin32_t's to a vector of 64 unsigned chars.
    Splits a uin32_t into bytes in big endian fashion.
*/
std::vector<unsigned char> makeBlock(const uint32_t* src)
{
    std::vector<unsigned char> block;
    for(int i = 0; i < 16; i++) {
        uint32_t w = src[i];
        block.push_back(w >> 24);
        block.push_back(w >> 16);
        block.push_back(w >> 8);
        block.push_back(w);
    }
    assert(64 == block.size());
    return block;
}

} // end anonymous namespace


TEST(sha1, processBlockInternal)
{
    for(int i = 0; i < 3; i++) {

        BlockTestCase& testCase = blockTestCases[i];
        std::vector<unsigned char> block = makeBlock(testCase.W);

        std::vector<uint32_t> H(5);
        memcpy( &H[0], testCase.Hin, sizeof(uint32_t) * H.size() );
        sha1::Sha1::processBlockInternal((const unsigned char*)&block[0], &H[0]);

        EXPECT_TRUE(0 == memcmp(testCase.Hout, &H[0], sizeof(uint32_t)*H.size() ) );
    }
}

namespace sha1 {
    extern const std::vector<std::pair<std::string, std::string>> testCases;
}

TEST(sha1, fulltests)
{
    for(const std::pair<std::string, std::string> testCase: sha1::testCases) {
        sha1::Sha1 sha1;
        std::vector<char> result(20);
        sha1.processStrFull(testCase.second.c_str(), testCase.second.length(), 0, &result[0]);
        std::stringstream resulthex;
        resulthex << std::hex;
        for(int i = 0; i < 20; i++) {
            resulthex << std::setfill('0') << std::setw(2) << ((int)(unsigned char)result[i]);
        }
        EXPECT_STREQ(testCase.first.c_str(), resulthex.str().c_str());
    }
}



