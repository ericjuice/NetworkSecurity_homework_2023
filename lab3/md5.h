#ifndef ZJX_MD5_H
#define ZJX_MD5_H

#include <stdio.h>
#include <string.h>
#include <string>
#include <cstring>

using namespace std;

// some define
typedef unsigned int MD5_u32;
#define F(x, y, z) ((z) ^ ((x) & ((y) ^ (z))))
#define G(x, y, z) ((y) ^ ((z) & ((x) ^ (y))))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | ~(z)))
#define STEP(f, a, b, c, d, x, t, s)                         \
    (a) += f((b), (c), (d)) + (x) + (t);                     \
    (a) = (((a) << (s)) | (((a)&0xffffffff) >> (32 - (s)))); \
    (a) += (b);

#define SET(n) \
    (*(MD5_u32 *)&ptr[(n)*4])
#define GET(n) \
    SET(n)

typedef struct
{
    MD5_u32 lo, hi;
    MD5_u32 a, b, c, d;
    unsigned char buffer[64];
    MD5_u32 block[16];
} MD5_CTX;

class MyMD5
{
    string version;
    MD5_CTX ctx_buffer;
    void MD5_Init(MD5_CTX *ctx);
    void MD5_Update(MD5_CTX *ctx, const void *data, unsigned long size);
    void MD5_Final(unsigned char *result, MD5_CTX *ctx);
    void md5bin(const void *dat, size_t len, unsigned char out[16]);
    char hb2hex(unsigned char hb);
    const void *body(MD5_CTX *ctx, const void *data, unsigned long size);
public:
    MyMD5()
    {
        this->version = "1.0";
    };
    string get_version()
    {
        return version;
    }

    // below are interfaces
    std::string md5(std::string dat);
    std::string md5(const void* dat, size_t len);
    std::string md5file(const char* filename);
    std::string md5file(std::FILE* file);
    static void print_help();
};

#endif