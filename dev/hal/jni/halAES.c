#include "halHeader.h"
#include "protocol.h"

static int lelinkPadding(uint8_t *data, uint32_t len, uint32_t maxLen, int isPKCS5)
{
    if (isPKCS5)
    {
        int i;
        uint8_t padding = 16;
        
        if ( len % 16 )
            padding = 16 - ( len % 16 );
        
        if ( ( len + padding ) > maxLen )
            return LELINK_ERR_ENCINFO_ERR;
        
        for ( i=0; i<padding; i++ )
            data[len + i] = padding;
        return len + padding;
    }
    else
    {
        len = (len + 15) & 0xfffffff0;
        if (len > maxLen)
            return LELINK_ERR_ENCINFO_ERR;
        return len;
    }
}

static int lelinkUnPadding(uint8_t *data, uint32_t len, int isPKCS5)
{
    if ( len % 16 )
        return LELINK_ERR_ENCINFO_ERR;
    
    if (isPKCS5)
    {
        int i;
        uint8_t padding;
        
        padding = data[len - 1];
        for (i=0; i<padding; i++)
        {
            if (data[len - 1 - i] != padding)
                return LELINK_ERR_RECV_DATA_ERR;
        }
        
        return len - padding;
    }
    else
    {
        return len;
    }
}

typedef struct
{
    int nr;                     /*!<  number of rounds  */
    unsigned long *rk;          /*!<  AES round keys    */
    unsigned long buf[68];      /*!<  unaligned data    */
}lelinkEnc2Context;

/*
 * 32-bit integer manipulation macros (little endian)
 */
#ifndef LELINK_GET_ULONG_LE
#define LELINK_GET_ULONG_LE(n,b,i)                             \
{                                                       \
(n) = ( (unsigned long) (b)[(i)    ]       )        \
| ( (unsigned long) (b)[(i) + 1] <<  8 )        \
| ( (unsigned long) (b)[(i) + 2] << 16 )        \
| ( (unsigned long) (b)[(i) + 3] << 24 );       \
}
#endif

#ifndef LELINK_PUT_ULONG_LE
#define LELINK_PUT_ULONG_LE(n,b,i)                             \
{                                                       \
(b)[(i)    ] = (unsigned char) ( (n)       );       \
(b)[(i) + 1] = (unsigned char) ( (n) >>  8 );       \
(b)[(i) + 2] = (unsigned char) ( (n) >> 16 );       \
(b)[(i) + 3] = (unsigned char) ( (n) >> 24 );       \
}
#endif

/*
 * Forward S-box & tables
 */
static unsigned char FSb[256];
static unsigned long FT0[256];
static unsigned long FT1[256];
static unsigned long FT2[256];
static unsigned long FT3[256];

/*
 * Reverse S-box & tables
 */
static unsigned char RSb[256];
static unsigned long RT0[256];
static unsigned long RT1[256];
static unsigned long RT2[256];
static unsigned long RT3[256];

/*
 * Round constants
 */
static unsigned long RCON[10];

/*
 * Tables generation code
 */
#define JOYLINKROTL8(x) ( ( x << 8 ) & 0xFFFFFFFF ) | ( x >> 24 )
#define JOYLINKXTIME(x) ( ( x << 1 ) ^ ( ( x & 0x80 ) ? 0x1B : 0x00 ) )
#define JOYLINKMUL(x,y) ( ( x && y ) ? pow[(log[x]+log[y]) % 255] : 0 )

static int lelink_enc2_init_done = 0;

static void lelink_enc2_gen_tables( void )
{
    int i, x, y, z;
    int pow[256];
    int log[256];
    
    /*
     * compute pow and log tables over GF(2^8)
     */
    for( i = 0, x = 1; i < 256; i++ )
    {
        pow[i] = x;
        log[x] = i;
        x = ( x ^ JOYLINKXTIME( x ) ) & 0xFF;
    }
    
    /*
     * calculate the round constants
     */
    for( i = 0, x = 1; i < 10; i++ )
    {
        RCON[i] = (unsigned long) x;
        x = JOYLINKXTIME( x ) & 0xFF;
    }
    
    /*
     * generate the forward and reverse S-boxes
     */
    FSb[0x00] = 0x63;
    RSb[0x63] = 0x00;
    
    for( i = 1; i < 256; i++ )
    {
        x = pow[255 - log[i]];
        
        y  = x; y = ( (y << 1) | (y >> 7) ) & 0xFF;
        x ^= y; y = ( (y << 1) | (y >> 7) ) & 0xFF;
        x ^= y; y = ( (y << 1) | (y >> 7) ) & 0xFF;
        x ^= y; y = ( (y << 1) | (y >> 7) ) & 0xFF;
        x ^= y ^ 0x63;
        
        FSb[i] = (unsigned char) x;
        RSb[x] = (unsigned char) i;
    }
    
    /*
     * generate the forward and reverse tables
     */
    for( i = 0; i < 256; i++ )
    {
        x = FSb[i];
        y = JOYLINKXTIME( x ) & 0xFF;
        z =  ( y ^ x ) & 0xFF;
        
        FT0[i] = ( (unsigned long) y       ) ^
        ( (unsigned long) x <<  8 ) ^
        ( (unsigned long) x << 16 ) ^
        ( (unsigned long) z << 24 );
        
        FT1[i] = JOYLINKROTL8( FT0[i] );
        FT2[i] = JOYLINKROTL8( FT1[i] );
        FT3[i] = JOYLINKROTL8( FT2[i] );
        
        x = RSb[i];
        
        RT0[i] = ( (unsigned long) JOYLINKMUL( 0x0E, x )       ) ^
        ( (unsigned long) JOYLINKMUL( 0x09, x ) <<  8 ) ^
        ( (unsigned long) JOYLINKMUL( 0x0D, x ) << 16 ) ^
        ( (unsigned long) JOYLINKMUL( 0x0B, x ) << 24 );
        
        RT1[i] = JOYLINKROTL8( RT0[i] );
        RT2[i] = JOYLINKROTL8( RT1[i] );
        RT3[i] = JOYLINKROTL8( RT2[i] );
    }
}

/*
 * AES key schedule (encryption)
 */
static int lelink_enc2_setkey_enc(lelinkEnc2Context *ctx, const unsigned char *key, unsigned int keysize)
{
    unsigned int i;
    unsigned long *RK;
    
    if( lelink_enc2_init_done == 0 )
    {
        lelink_enc2_gen_tables();
        lelink_enc2_init_done = 1;
    }
    
    switch( keysize )
    {
        case 128: ctx->nr = 10; break;
        case 192: ctx->nr = 12; break;
        case 256: ctx->nr = 14; break;
        default : return( -1 );
    }
    
    ctx->rk = RK = ctx->buf;
    
    for( i = 0; i < (keysize >> 5); i++ )
    {
        LELINK_GET_ULONG_LE( RK[i], key, i << 2 );
    }
    
    switch( ctx->nr )
    {
        case 10:
            
            for( i = 0; i < 10; i++, RK += 4 )
            {
                RK[4]  = RK[0] ^ RCON[i] ^
                ( (unsigned long) FSb[ ( RK[3] >>  8 ) & 0xFF ]       ) ^
                ( (unsigned long) FSb[ ( RK[3] >> 16 ) & 0xFF ] <<  8 ) ^
                ( (unsigned long) FSb[ ( RK[3] >> 24 ) & 0xFF ] << 16 ) ^
                ( (unsigned long) FSb[ ( RK[3]       ) & 0xFF ] << 24 );
                
                RK[5]  = RK[1] ^ RK[4];
                RK[6]  = RK[2] ^ RK[5];
                RK[7]  = RK[3] ^ RK[6];
            }
            break;
            
        case 12:
            
            for( i = 0; i < 8; i++, RK += 6 )
            {
                RK[6]  = RK[0] ^ RCON[i] ^
                ( (unsigned long) FSb[ ( RK[5] >>  8 ) & 0xFF ]       ) ^
                ( (unsigned long) FSb[ ( RK[5] >> 16 ) & 0xFF ] <<  8 ) ^
                ( (unsigned long) FSb[ ( RK[5] >> 24 ) & 0xFF ] << 16 ) ^
                ( (unsigned long) FSb[ ( RK[5]       ) & 0xFF ] << 24 );
                
                RK[7]  = RK[1] ^ RK[6];
                RK[8]  = RK[2] ^ RK[7];
                RK[9]  = RK[3] ^ RK[8];
                RK[10] = RK[4] ^ RK[9];
                RK[11] = RK[5] ^ RK[10];
            }
            break;
            
        case 14:
            
            for( i = 0; i < 7; i++, RK += 8 )
            {
                RK[8]  = RK[0] ^ RCON[i] ^
                ( (unsigned long) FSb[ ( RK[7] >>  8 ) & 0xFF ]       ) ^
                ( (unsigned long) FSb[ ( RK[7] >> 16 ) & 0xFF ] <<  8 ) ^
                ( (unsigned long) FSb[ ( RK[7] >> 24 ) & 0xFF ] << 16 ) ^
                ( (unsigned long) FSb[ ( RK[7]       ) & 0xFF ] << 24 );
                
                RK[9]  = RK[1] ^ RK[8];
                RK[10] = RK[2] ^ RK[9];
                RK[11] = RK[3] ^ RK[10];
                
                RK[12] = RK[4] ^
                ( (unsigned long) FSb[ ( RK[11]       ) & 0xFF ]       ) ^
                ( (unsigned long) FSb[ ( RK[11] >>  8 ) & 0xFF ] <<  8 ) ^
                ( (unsigned long) FSb[ ( RK[11] >> 16 ) & 0xFF ] << 16 ) ^
                ( (unsigned long) FSb[ ( RK[11] >> 24 ) & 0xFF ] << 24 );
                
                RK[13] = RK[5] ^ RK[12];
                RK[14] = RK[6] ^ RK[13];
                RK[15] = RK[7] ^ RK[14];
            }
            break;
            
        default:
            
            break;
    }
    
    return( 0 );
}

/*
 * AES key schedule (decryption)
 */
static int lelink_enc2_setkey_dec( lelinkEnc2Context *ctx, const unsigned char *key, unsigned int keysize )
{
    int i, j;
    lelinkEnc2Context cty;
    unsigned long *RK;
    unsigned long *SK;
    int ret;
    
    switch( keysize )
    {
        case 128: ctx->nr = 10; break;
        case 192: ctx->nr = 12; break;
        case 256: ctx->nr = 14; break;
        default : return( -1 );
    }
    
    ctx->rk = RK = ctx->buf;
    
    ret = lelink_enc2_setkey_enc( &cty, key, keysize );
    if( ret != 0 )
        return( ret );
    
    SK = cty.rk + cty.nr * 4;
    
    *RK++ = *SK++;
    *RK++ = *SK++;
    *RK++ = *SK++;
    *RK++ = *SK++;
    
    for( i = ctx->nr - 1, SK -= 8; i > 0; i--, SK -= 8 )
    {
        for( j = 0; j < 4; j++, SK++ )
        {
            *RK++ = RT0[ FSb[ ( *SK       ) & 0xFF ] ] ^
            RT1[ FSb[ ( *SK >>  8 ) & 0xFF ] ] ^
            RT2[ FSb[ ( *SK >> 16 ) & 0xFF ] ] ^
            RT3[ FSb[ ( *SK >> 24 ) & 0xFF ] ];
        }
    }
    
    *RK++ = *SK++;
    *RK++ = *SK++;
    *RK++ = *SK++;
    *RK++ = *SK++;
    
    memset( &cty, 0, sizeof( lelinkEnc2Context ) );
    
    return( 0 );
}

#define LELINK_AES_FROUND(X0,X1,X2,X3,Y0,Y1,Y2,Y3)     \
{                                               \
X0 = *RK++ ^ FT0[ ( Y0       ) & 0xFF ] ^   \
FT1[ ( Y1 >>  8 ) & 0xFF ] ^   \
FT2[ ( Y2 >> 16 ) & 0xFF ] ^   \
FT3[ ( Y3 >> 24 ) & 0xFF ];    \
\
X1 = *RK++ ^ FT0[ ( Y1       ) & 0xFF ] ^   \
FT1[ ( Y2 >>  8 ) & 0xFF ] ^   \
FT2[ ( Y3 >> 16 ) & 0xFF ] ^   \
FT3[ ( Y0 >> 24 ) & 0xFF ];    \
\
X2 = *RK++ ^ FT0[ ( Y2       ) & 0xFF ] ^   \
FT1[ ( Y3 >>  8 ) & 0xFF ] ^   \
FT2[ ( Y0 >> 16 ) & 0xFF ] ^   \
FT3[ ( Y1 >> 24 ) & 0xFF ];    \
\
X3 = *RK++ ^ FT0[ ( Y3       ) & 0xFF ] ^   \
FT1[ ( Y0 >>  8 ) & 0xFF ] ^   \
FT2[ ( Y1 >> 16 ) & 0xFF ] ^   \
FT3[ ( Y2 >> 24 ) & 0xFF ];    \
}

#define LELINK_AES_RROUND(X0,X1,X2,X3,Y0,Y1,Y2,Y3)     \
{                                               \
X0 = *RK++ ^ RT0[ ( Y0       ) & 0xFF ] ^   \
RT1[ ( Y3 >>  8 ) & 0xFF ] ^   \
RT2[ ( Y2 >> 16 ) & 0xFF ] ^   \
RT3[ ( Y1 >> 24 ) & 0xFF ];    \
\
X1 = *RK++ ^ RT0[ ( Y1       ) & 0xFF ] ^   \
RT1[ ( Y0 >>  8 ) & 0xFF ] ^   \
RT2[ ( Y3 >> 16 ) & 0xFF ] ^   \
RT3[ ( Y2 >> 24 ) & 0xFF ];    \
\
X2 = *RK++ ^ RT0[ ( Y2       ) & 0xFF ] ^   \
RT1[ ( Y1 >>  8 ) & 0xFF ] ^   \
RT2[ ( Y0 >> 16 ) & 0xFF ] ^   \
RT3[ ( Y3 >> 24 ) & 0xFF ];    \
\
X3 = *RK++ ^ RT0[ ( Y3       ) & 0xFF ] ^   \
RT1[ ( Y2 >>  8 ) & 0xFF ] ^   \
RT2[ ( Y1 >> 16 ) & 0xFF ] ^   \
RT3[ ( Y0 >> 24 ) & 0xFF ];    \
}

/*
 * AES-ECB block encryption/decryption
 */
static int lelink_enc2_crypt_ecb( lelinkEnc2Context *ctx,
                             int mode,
                             const unsigned char input[16],
                             unsigned char output[16] )
{
    int i;
    unsigned long *RK, X0, X1, X2, X3, Y0, Y1, Y2, Y3;
    
    RK = ctx->rk;
    
    LELINK_GET_ULONG_LE( X0, input,  0 ); X0 ^= *RK++;
    LELINK_GET_ULONG_LE( X1, input,  4 ); X1 ^= *RK++;
    LELINK_GET_ULONG_LE( X2, input,  8 ); X2 ^= *RK++;
    LELINK_GET_ULONG_LE( X3, input, 12 ); X3 ^= *RK++;
    
    if (!mode)
    {
        for( i = (ctx->nr >> 1) - 1; i > 0; i-- )
        {
            LELINK_AES_RROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );
            LELINK_AES_RROUND( X0, X1, X2, X3, Y0, Y1, Y2, Y3 );
        }
        
        LELINK_AES_RROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );
        
        X0 = *RK++ ^ \
        ( (unsigned long) RSb[ ( Y0       ) & 0xFF ]       ) ^
        ( (unsigned long) RSb[ ( Y3 >>  8 ) & 0xFF ] <<  8 ) ^
        ( (unsigned long) RSb[ ( Y2 >> 16 ) & 0xFF ] << 16 ) ^
        ( (unsigned long) RSb[ ( Y1 >> 24 ) & 0xFF ] << 24 );
        
        X1 = *RK++ ^ \
        ( (unsigned long) RSb[ ( Y1       ) & 0xFF ]       ) ^
        ( (unsigned long) RSb[ ( Y0 >>  8 ) & 0xFF ] <<  8 ) ^
        ( (unsigned long) RSb[ ( Y3 >> 16 ) & 0xFF ] << 16 ) ^
        ( (unsigned long) RSb[ ( Y2 >> 24 ) & 0xFF ] << 24 );
        
        X2 = *RK++ ^ \
        ( (unsigned long) RSb[ ( Y2       ) & 0xFF ]       ) ^
        ( (unsigned long) RSb[ ( Y1 >>  8 ) & 0xFF ] <<  8 ) ^
        ( (unsigned long) RSb[ ( Y0 >> 16 ) & 0xFF ] << 16 ) ^
        ( (unsigned long) RSb[ ( Y3 >> 24 ) & 0xFF ] << 24 );
        
        X3 = *RK++ ^ \
        ( (unsigned long) RSb[ ( Y3       ) & 0xFF ]       ) ^
        ( (unsigned long) RSb[ ( Y2 >>  8 ) & 0xFF ] <<  8 ) ^
        ( (unsigned long) RSb[ ( Y1 >> 16 ) & 0xFF ] << 16 ) ^
        ( (unsigned long) RSb[ ( Y0 >> 24 ) & 0xFF ] << 24 );
    }
    else /* LELINK_AES_ENCRYPT */
    {
        for( i = (ctx->nr >> 1) - 1; i > 0; i-- )
        {
            LELINK_AES_FROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );
            LELINK_AES_FROUND( X0, X1, X2, X3, Y0, Y1, Y2, Y3 );
        }
        
        LELINK_AES_FROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );
        
        X0 = *RK++ ^ \
        ( (unsigned long) FSb[ ( Y0       ) & 0xFF ]       ) ^
        ( (unsigned long) FSb[ ( Y1 >>  8 ) & 0xFF ] <<  8 ) ^
        ( (unsigned long) FSb[ ( Y2 >> 16 ) & 0xFF ] << 16 ) ^
        ( (unsigned long) FSb[ ( Y3 >> 24 ) & 0xFF ] << 24 );
        
        X1 = *RK++ ^ \
        ( (unsigned long) FSb[ ( Y1       ) & 0xFF ]       ) ^
        ( (unsigned long) FSb[ ( Y2 >>  8 ) & 0xFF ] <<  8 ) ^
        ( (unsigned long) FSb[ ( Y3 >> 16 ) & 0xFF ] << 16 ) ^
        ( (unsigned long) FSb[ ( Y0 >> 24 ) & 0xFF ] << 24 );
        
        X2 = *RK++ ^ \
        ( (unsigned long) FSb[ ( Y2       ) & 0xFF ]       ) ^
        ( (unsigned long) FSb[ ( Y3 >>  8 ) & 0xFF ] <<  8 ) ^
        ( (unsigned long) FSb[ ( Y0 >> 16 ) & 0xFF ] << 16 ) ^
        ( (unsigned long) FSb[ ( Y1 >> 24 ) & 0xFF ] << 24 );
        
        X3 = *RK++ ^ \
        ( (unsigned long) FSb[ ( Y3       ) & 0xFF ]       ) ^
        ( (unsigned long) FSb[ ( Y0 >>  8 ) & 0xFF ] <<  8 ) ^
        ( (unsigned long) FSb[ ( Y1 >> 16 ) & 0xFF ] << 16 ) ^
        ( (unsigned long) FSb[ ( Y2 >> 24 ) & 0xFF ] << 24 );
    }
    
    LELINK_PUT_ULONG_LE( X0, output,  0 );
    LELINK_PUT_ULONG_LE( X1, output,  4 );
    LELINK_PUT_ULONG_LE( X2, output,  8 );
    LELINK_PUT_ULONG_LE( X3, output, 12 );
    
    return( 0 );
}

/*
 * AES-CBC buffer encryption/decryption
 */
static int lelink_enc2_crypt_cbc( lelinkEnc2Context *ctx,
                             int mode,
                             size_t length,
                             unsigned char iv[16],
                             const unsigned char *input,
                             unsigned char *output )
{
    int i;
    unsigned char temp[16];
    
    if( length % 16 )
        return( -1 );
    
    if (!mode)
    {
        while( length > 0 )
        {
            memcpy( temp, input, 16 );
            lelink_enc2_crypt_ecb( ctx, mode, input, output );
            
            for( i = 0; i < 16; i++ )
                output[i] = (unsigned char)( output[i] ^ iv[i] );
            
            memcpy( iv, temp, 16 );
            
            input  += 16;
            output += 16;
            length -= 16;
        }
    }
    else
    {
        while( length > 0 )
        {
            for( i = 0; i < 16; i++ )
                output[i] = (unsigned char)( input[i] ^ iv[i] );
            
            lelink_enc2_crypt_ecb( ctx, mode, output, output );
            memcpy( iv, output, 16 );
            
            input  += 16;
            output += 16;
            length -= 16;
        }
    }
    
    return( 0 );
}

int halAESInit(void) {
    return 0;
}
//AES crypt
int halAES(uint8_t *key, uint32_t keyLen, uint8_t *iv, uint8_t *data, uint32_t *len, uint32_t maxLen, int isPKCS5, int type)
{
    int dl;
    int ret;
    lelinkEnc2Context ctx;
    
    memset(&ctx, 0, sizeof(lelinkEnc2Context));
    if (!type)
    {
        lelink_enc2_setkey_dec(&ctx, key, keyLen);
        ret = lelink_enc2_crypt_cbc(&ctx, type, (size_t)(*len), iv, data, data);
        dl = lelinkUnPadding(data, (uint32_t)(*len), isPKCS5);
        if (dl < 0)
            return dl;
    }
    else
    {
        dl = lelinkPadding(data, *len, maxLen, isPKCS5);
        if (dl < 0)
            return dl;
        lelink_enc2_setkey_enc(&ctx, key, keyLen);
        ret = lelink_enc2_crypt_cbc(&ctx, type, (size_t)dl, iv, data, data);
    }
    
    *len = dl;
    
    return ret;
}
