#include "dlb_aes.h"

/* Perform doubling in Galois Field GF(2^8) using the irreducible polynomial
   x^8+x^4+x^3+x+1 */
static unsigned char AES_xtime(uint32_t x)
{
	return (x&0x80) ? (x<<1)^0x1b : x<<1;
}

inline static uint16_t crypto_htons(uint16_t x)
{
    return (
            ((x & 0xff)   << 8) | 
            ((x & 0xff00) >> 8)
           );
}
 
inline static uint16_t crypto_ntohs(uint16_t x)
{
    return (
            ((x & 0xff)   << 8) | 
            ((x & 0xff00) >> 8)
           );
}

inline static uint32_t crypto_htonl(uint32_t x)
{
    return (
            ((x & 0xff)         << 24) | 
            ((x & 0xff00)       << 8)  | 
            ((x & 0xff0000UL)   >> 8)  | 
            ((x & 0xff000000UL) >> 24)
           );
}

inline static uint32_t crypto_ntohl(uint32_t x)
{
    return (
            ((x & 0xff)         << 24) | 
            ((x & 0xff00)       << 8)  | 
            ((x & 0xff0000UL)   >> 8)  | 
            ((x & 0xff000000UL) >> 24)
           );
}
/**
 * Encrypt a single block (16 bytes) of data
 */
void AES::encrypt(uint32_t *data)
{
    /* To make this code smaller, generate the sbox entries on the fly.
     * This will have a really heavy effect upon performance.
     */
    uint32_t tmp[4];
    uint32_t tmp1, old_a0, a0, a1, a2, a3, row;
    int curr_rnd;
    int rounds = _rounds; 
    const uint32_t *k = _ks;

    /* Pre-round key addition */
    for (row = 0; row < 4; row++)
        data[row] ^= *(k++);

    /* Encrypt one block. */
    for (curr_rnd = 0; curr_rnd < rounds; curr_rnd++)
    {
        /* Perform ByteSub and ShiftRow operations together */
        for (row = 0; row < 4; row++)
        {
            a0 = (uint32_t)aes_sbox[(data[row%4]>>24)&0xFF];
            a1 = (uint32_t)aes_sbox[(data[(row+1)%4]>>16)&0xFF];
            a2 = (uint32_t)aes_sbox[(data[(row+2)%4]>>8)&0xFF]; 
            a3 = (uint32_t)aes_sbox[(data[(row+3)%4])&0xFF];

            /* Perform MixColumn iff not last round */
            if (curr_rnd < (rounds - 1))
            {
                tmp1 = a0 ^ a1 ^ a2 ^ a3;
                old_a0 = a0;
                a0 ^= tmp1 ^ AES_xtime(a0 ^ a1);
                a1 ^= tmp1 ^ AES_xtime(a1 ^ a2);
                a2 ^= tmp1 ^ AES_xtime(a2 ^ a3);
                a3 ^= tmp1 ^ AES_xtime(a3 ^ old_a0);
            }

            tmp[row] = ((a0 << 24) | (a1 << 16) | (a2 << 8) | a3);
        }

        /* KeyAddition - note that it is vital that this loop is separate from
           the MixColumn operation, which must be atomic...*/ 
        for (row = 0; row < 4; row++)
            data[row] = tmp[row] ^ *(k++);
    }
#if defined ESP8266
    ESP.wdtFeed();
#endif
}

/**
 * Decrypt a single block (16 bytes) of data
 */
void AES::decrypt(uint32_t *data)
{ 
    uint32_t tmp[4];
    uint32_t xt0,xt1,xt2,xt3,xt4,xt5,xt6;
    uint32_t a0, a1, a2, a3, row;
    int curr_rnd;
    int rounds = _rounds;
    const uint32_t *k = _ks + ((rounds+1)*4);

    /* pre-round key addition */
    for (row=4; row > 0;row--)
        data[row-1] ^= *(--k);

    /* Decrypt one block */
    for (curr_rnd = 0; curr_rnd < rounds; curr_rnd++)
    {
        /* Perform ByteSub and ShiftRow operations together */
        for (row = 4; row > 0; row--)
        {
            a0 = aes_isbox[(data[(row+3)%4]>>24)&0xFF];
            a1 = aes_isbox[(data[(row+2)%4]>>16)&0xFF];
            a2 = aes_isbox[(data[(row+1)%4]>>8)&0xFF];
            a3 = aes_isbox[(data[row%4])&0xFF];

            /* Perform MixColumn iff not last round */
            if (curr_rnd<(rounds-1))
            {
                /* The MDS cofefficients (0x09, 0x0B, 0x0D, 0x0E)
                   are quite large compared to encryption; this 
                   operation slows decryption down noticeably. */
                xt0 = AES_xtime(a0^a1);
                xt1 = AES_xtime(a1^a2);
                xt2 = AES_xtime(a2^a3);
                xt3 = AES_xtime(a3^a0);
                xt4 = AES_xtime(xt0^xt1);
                xt5 = AES_xtime(xt1^xt2);
                xt6 = AES_xtime(xt4^xt5);

                xt0 ^= a1^a2^a3^xt4^xt6;
                xt1 ^= a0^a2^a3^xt5^xt6;
                xt2 ^= a0^a1^a3^xt4^xt6;
                xt3 ^= a0^a1^a2^xt5^xt6;
                tmp[row-1] = ((xt0<<24)|(xt1<<16)|(xt2<<8)|xt3);
            }
            else
                tmp[row-1] = ((a0<<24)|(a1<<16)|(a2<<8)|a3);
        }

        for (row = 4; row > 0; row--)
            data[row-1] = tmp[row-1] ^ *(--k);
    }
#if defined ESP8266
    ESP.wdtFeed();
#endif
}

AES::AES(const uint8_t *key, const uint8_t *iv, AES_MODE mode, CIPHER_MODE cipherMode)
{
    _cipherMode = cipherMode;
    
    int i, ii;
    uint32_t *W, tmp, tmp2;
    const unsigned char *ip;
    int words;

    _arr_pad[0] = 0x01;
    _arr_pad[1] = 0x02;
    _arr_pad[2] = 0x03;
    _arr_pad[3] = 0x04;
    _arr_pad[4] = 0x05;
    _arr_pad[5] = 0x06;
    _arr_pad[6] = 0x07;
    _arr_pad[7] = 0x08;
    _arr_pad[8] = 0x09;
    _arr_pad[9] = 0x0a;
    _arr_pad[10] = 0x0b;
    _arr_pad[11] = 0x0c;
    _arr_pad[12] = 0x0d;
    _arr_pad[13] = 0x0e;
    _arr_pad[14] = 0x0f;

    switch (mode)
    {
        case AES_MODE_128:
            i = 10;
            words = 4;
            break;

        case AES_MODE_256:
            i = 14;
            words = 8;
            break;

        default:        /* fail silently */
            return;
    }

    _rounds = i;
    _key_size = words;
    W = _ks;
    for (i = 0; i < words; i+=2)
    {
        W[i+0]=	((uint32_t)key[ 0]<<24)|
            ((uint32_t)key[ 1]<<16)|
            ((uint32_t)key[ 2]<< 8)|
            ((uint32_t)key[ 3]    );
        W[i+1]=	((uint32_t)key[ 4]<<24)|
            ((uint32_t)key[ 5]<<16)|
            ((uint32_t)key[ 6]<< 8)|
            ((uint32_t)key[ 7]    );
        key += 8;
    }

    ip = Rcon;
    ii = 4 * (_rounds+1);
    for (i = words; i<ii; i++)
    {
        tmp = W[i-1];

        if ((i % words) == 0)
        {
            tmp2 =(uint32_t)aes_sbox[(tmp    )&0xff]<< 8;
            tmp2|=(uint32_t)aes_sbox[(tmp>> 8)&0xff]<<16;
            tmp2|=(uint32_t)aes_sbox[(tmp>>16)&0xff]<<24;
            tmp2|=(uint32_t)aes_sbox[(tmp>>24)     ];
            tmp=tmp2^(((unsigned int)*ip)<<24);
            ip++;
        }

        if ((words == 8) && ((i % words) == 4))
        {
            tmp2 =(uint32_t)aes_sbox[(tmp    )&0xff]    ;
            tmp2|=(uint32_t)aes_sbox[(tmp>> 8)&0xff]<< 8;
            tmp2|=(uint32_t)aes_sbox[(tmp>>16)&0xff]<<16;
            tmp2|=(uint32_t)aes_sbox[(tmp>>24)     ]<<24;
            tmp=tmp2;
        }

        W[i]=W[i-words]^tmp;
    }

    /* copy the iv across */
    memcpy(_iv, iv, 16);
    
    /* Do we need to convert the key */
    if (_cipherMode == CIPHER_DECRYPT)
    {
        convertKey();
    }
#if defined ESP8266
    ESP.wdtFeed();
#endif
}

int AES::getSize()
{
    return _size;
}

void AES::setSize(int size)
{
    _size = size;
}

int AES::calcSizeAndPad(int in_size)
{
    in_size++; // +1 for null terminater on input string
    int buf = round(in_size / AES_BLOCKSIZE) * AES_BLOCKSIZE;
    _size = (buf <= in_size) ? buf + AES_BLOCKSIZE : buf;
    _pad_size = _size - in_size;
    return _size;
}

void AES::padPlaintext(const uint8_t* in, uint8_t* out)
{
    memcpy(out, in, _size);
    for (int i = _size - _pad_size; i < _size; i++)
    {
        out[i] = _arr_pad[_pad_size - 1];
    }
}

bool AES::checkPad(uint8_t* in, int lsize)
{
    if (in[lsize-1] <= 0x0f)
    {
        int lpad = (int)in[lsize-1];
        for (int i = lsize - 1; i >= lsize-lpad; i--)
        {
            if (_arr_pad[lpad - 1] != in[i])
            {
                return false;
            }
        }
    } 
    else 
    {
        return true;
    }
    return true;
}

void AES::processNoPad(const uint8_t *in, uint8_t *out, int length)
{
    if (_cipherMode == CIPHER_ENCRYPT)
    {
        encryptCBC(in, out, length);
    } 
    else 
    {
        decryptCBC(in, out, length);
    }
}

void AES::process(const uint8_t *in, uint8_t *out, int length)
{
    if (_cipherMode == CIPHER_ENCRYPT)
    {
        calcSizeAndPad(length);
        uint8_t in_pad[getSize()];
        padPlaintext(in, in_pad);
        encryptCBC(in_pad, out, getSize());
    } 
    else 
    {
        decryptCBC(in, out, length);
    }
}

void AES::encryptCBC(const uint8_t *in, uint8_t *out, int length)
{
    int i;
    uint32_t tin[4], tout[4], iv[4];

    memcpy(iv, _iv, AES_IV_SIZE);
    for (i = 0; i < 4; i++)
        tout[i] = crypto_ntohl(iv[i]);

    for (length -= AES_BLOCKSIZE; length >= 0; length -= AES_BLOCKSIZE)
    {
        uint32_t msg_32[4];
        uint32_t out_32[4];
        memcpy(msg_32, in, AES_BLOCKSIZE);
        in += AES_BLOCKSIZE;

        for (i = 0; i < 4; i++)
            tin[i] = crypto_ntohl(msg_32[i])^tout[i];

        AES::encrypt(tin);

        for (i = 0; i < 4; i++)
        {
            tout[i] = tin[i]; 
            out_32[i] = crypto_htonl(tout[i]);
        }

        memcpy(out, out_32, AES_BLOCKSIZE);
        out += AES_BLOCKSIZE;
    }

    for (i = 0; i < 4; i++)
        iv[i] = crypto_htonl(tout[i]);
    memcpy(_iv, iv, AES_IV_SIZE);
#if defined ESP8266
    ESP.wdtFeed();
#endif
}

void AES::decryptCBC(const uint8_t *in, uint8_t *out, int length)
{
    int i;
    uint32_t tin[4], bufxor[4], tout[4], data[4], iv[4];

    memcpy(iv, _iv, AES_IV_SIZE);
    for (i = 0; i < 4; i++)
        bufxor[i] = crypto_ntohl(iv[i]);

    for (length -= 16; length >= 0; length -= 16)
    {
        uint32_t msg_32[4];
        uint32_t out_32[4];
        memcpy(msg_32, in, AES_BLOCKSIZE);
        in += AES_BLOCKSIZE;

        for (i = 0; i < 4; i++)
        {
            tin[i] = crypto_ntohl(msg_32[i]);
            data[i] = tin[i];
        }

        AES::decrypt(data);

        for (i = 0; i < 4; i++)
        {
            tout[i] = data[i] ^ bufxor[i];
            bufxor[i] = tin[i];
            out_32[i] = crypto_htonl(tout[i]);
        }

        memcpy(out, out_32, AES_BLOCKSIZE);
        out += AES_BLOCKSIZE;
    }

    for (i = 0; i < 4; i++)
        iv[i] = crypto_htonl(bufxor[i]);
    memcpy(_iv, iv, AES_IV_SIZE);
#if defined ESP8266
    ESP.wdtFeed();
#endif
}

void AES::convertKey()
{
    int i;
    uint32_t *k,w,t1,t2,t3,t4;

    k = _ks;
    k += 4;

    for (i= _rounds*4; i > 4; i--)
    {
        w= *k;
        w = inv_mix_col(w,t1,t2,t3,t4);
        *k++ =w;
    }
}
