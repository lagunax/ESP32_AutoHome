//#include <stdio.h>
//#include <stdint.h>
//#include <string.h>
//#include <limits.h>
#include "upng.h"

#define FIRST_LENGTH_CODE_INDEX 257
#define LAST_LENGTH_CODE_INDEX 285
#define INVALID_CODE_INDEX UINT16_MAX

#define NUM_DEFLATE_CODE_SYMBOLS 288 /*256 literals, the end code, some length codes, and 2 unused codes */
#define NUM_DISTANCE_SYMBOLS 32      /*the distance codes have their own symbols, 30 used, 2 unused */
#define NUM_CODE_LENGTH_CODES 19     /*the code length codes. 0-15: code lengths, 16: copy previous 3-6 times, 17: 3-10 zeros, 18: 11-138 zeros */
#define MAX_SYMBOLS 288              /* largest number of symbols used by any tree type */

#define DEFLATE_CODE_BITLEN 15
#define DISTANCE_BITLEN 15
#define CODE_LENGTH_BITLEN 7
#define MAX_BIT_LENGTH 15 // bug? 15 /* largest bitlen used by any tree type */

#define DEFLATE_CODE_BUFFER_SIZE (NUM_DEFLATE_CODE_SYMBOLS * 2)
#define DISTANCE_BUFFER_SIZE (NUM_DISTANCE_SYMBOLS * 2)
#define CODE_LENGTH_BUFFER_SIZE (NUM_DISTANCE_SYMBOLS * 2)

typedef struct huffman_tree
{
    uint16_t *tree2d;
    uint16_t maxbitlen; /*maximum number of bits a single code can get */
    uint16_t numcodes;  /*number of symbols in the alphabet = number of codes */
} huffman_tree;

static const uint16_t LENGTH_BASE[29] = {/*the base lengths represented by codes 257-285 */
                                         3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59,
                                         67, 83, 99, 115, 131, 163, 195, 227, 258};

static const uint16_t LENGTH_EXTRA[29] = {/*the extra bits used by codes 257-285 (added to base length) */
                                          0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5,
                                          5, 5, 5, 0};

static const uint16_t DISTANCE_BASE[30] = {/*the base backwards distances (the bits of distance codes appear after length codes and use their own huffman tree) */
                                           1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513,
                                           769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577};

static const uint16_t DISTANCE_EXTRA[30] = {/*the extra bits of backwards distances (added to base) */
                                            0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10,
                                            11, 11, 12, 12, 13, 13};

static const uint16_t CLCL[NUM_CODE_LENGTH_CODES] /*the order in which "code length alphabet code lengths" are stored, out of this the huffman tree of the dynamic huffman tree lengths is generated */
    = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

static const uint16_t FIXED_DEFLATE_CODE_TREE[NUM_DEFLATE_CODE_SYMBOLS * 2] = {
    289, 370, 290, 307, 546, 291, 561, 292, 293, 300, 294, 297, 295, 296, 0, 1,
    2, 3, 298, 299, 4, 5, 6, 7, 301, 304, 302, 303, 8, 9, 10, 11, 305, 306, 12,
    13, 14, 15, 308, 339, 309, 324, 310, 317, 311, 314, 312, 313, 16, 17, 18,
    19, 315, 316, 20, 21, 22, 23, 318, 321, 319, 320, 24, 25, 26, 27, 322, 323,
    28, 29, 30, 31, 325, 332, 326, 329, 327, 328, 32, 33, 34, 35, 330, 331, 36,
    37, 38, 39, 333, 336, 334, 335, 40, 41, 42, 43, 337, 338, 44, 45, 46, 47,
    340, 355, 341, 348, 342, 345, 343, 344, 48, 49, 50, 51, 346, 347, 52, 53,
    54, 55, 349, 352, 350, 351, 56, 57, 58, 59, 353, 354, 60, 61, 62, 63, 356,
    363, 357, 360, 358, 359, 64, 65, 66, 67, 361, 362, 68, 69, 70, 71, 364,
    367, 365, 366, 72, 73, 74, 75, 368, 369, 76, 77, 78, 79, 371, 434, 372,
    403, 373, 388, 374, 381, 375, 378, 376, 377, 80, 81, 82, 83, 379, 380, 84,
    85, 86, 87, 382, 385, 383, 384, 88, 89, 90, 91, 386, 387, 92, 93, 94, 95,
    389, 396, 390, 393, 391, 392, 96, 97, 98, 99, 394, 395, 100, 101, 102, 103,
    397, 400, 398, 399, 104, 105, 106, 107, 401, 402, 108, 109, 110, 111, 404,
    419, 405, 412, 406, 409, 407, 408, 112, 113, 114, 115, 410, 411, 116, 117,
    118, 119, 413, 416, 414, 415, 120, 121, 122, 123, 417, 418, 124, 125, 126,
    127, 420, 427, 421, 424, 422, 423, 128, 129, 130, 131, 425, 426, 132, 133,
    134, 135, 428, 431, 429, 430, 136, 137, 138, 139, 432, 433, 140, 141, 142,
    143, 435, 483, 436, 452, 568, 437, 438, 445, 439, 442, 440, 441, 144, 145,
    146, 147, 443, 444, 148, 149, 150, 151, 446, 449, 447, 448, 152, 153, 154,
    155, 450, 451, 156, 157, 158, 159, 453, 468, 454, 461, 455, 458, 456, 457,
    160, 161, 162, 163, 459, 460, 164, 165, 166, 167, 462, 465, 463, 464, 168,
    169, 170, 171, 466, 467, 172, 173, 174, 175, 469, 476, 470, 473, 471, 472,
    176, 177, 178, 179, 474, 475, 180, 181, 182, 183, 477, 480, 478, 479, 184,
    185, 186, 187, 481, 482, 188, 189, 190, 191, 484, 515, 485, 500, 486, 493,
    487, 490, 488, 489, 192, 193, 194, 195, 491, 492, 196, 197, 198, 199, 494,
    497, 495, 496, 200, 201, 202, 203, 498, 499, 204, 205, 206, 207, 501, 508,
    502, 505, 503, 504, 208, 209, 210, 211, 506, 507, 212, 213, 214, 215, 509,
    512, 510, 511, 216, 217, 218, 219, 513, 514, 220, 221, 222, 223, 516, 531,
    517, 524, 518, 521, 519, 520, 224, 225, 226, 227, 522, 523, 228, 229, 230,
    231, 525, 528, 526, 527, 232, 233, 234, 235, 529, 530, 236, 237, 238, 239,
    532, 539, 533, 536, 534, 535, 240, 241, 242, 243, 537, 538, 244, 245, 246,
    247, 540, 543, 541, 542, 248, 249, 250, 251, 544, 545, 252, 253, 254, 255,
    547, 554, 548, 551, 549, 550, 256, 257, 258, 259, 552, 553, 260, 261, 262,
    263, 555, 558, 556, 557, 264, 265, 266, 267, 559, 560, 268, 269, 270, 271,
    562, 565, 563, 564, 272, 273, 274, 275, 566, 567, 276, 277, 278, 279, 569,
    572, 570, 571, 280, 281, 282, 283, 573, 574, 284, 285, 286, 287, 0, 0};

static const uint16_t FIXED_DISTANCE_TREE[NUM_DISTANCE_SYMBOLS * 2] = {
    33, 48, 34, 41, 35, 38, 36, 37, 0, 1, 2, 3, 39, 40, 4, 5, 6, 7, 42, 45, 43,
    44, 8, 9, 10, 11, 46, 47, 12, 13, 14, 15, 49, 56, 50, 53, 51, 52, 16, 17,
    18, 19, 54, 55, 20, 21, 22, 23, 57, 60, 58, 59, 24, 25, 26, 27, 61, 62, 28,
    29, 30, 31, 0, 0};

static unsigned char read_bit(unsigned long *bitpointer, const unsigned char *bitstream)
{
    unsigned char result = (unsigned char)((bitstream[(*bitpointer) >> 3] >> ((*bitpointer) & 0x7)) & 1);
    (*bitpointer)++;
    return result;
}

static unsigned read_bits(unsigned long *bitpointer, const unsigned char *bitstream, unsigned long nbits)
{
    unsigned result = 0, i;
    for (i = 0; i < nbits; i++)
        result |= ((unsigned)read_bit(bitpointer, bitstream)) << i;
    return result;
}

/* the buffer must be numcodes*2 in size! */
static void huffman_tree_init(huffman_tree *tree, uint16_t *buffer, uint16_t numcodes, uint16_t maxbitlen)
{
    tree->tree2d = buffer;

    tree->numcodes = numcodes;
    tree->maxbitlen = maxbitlen;
}

/*given the code lengths (as stored in the PNG file), generate the tree as defined by Deflate. maxbitlen is the maximum bits that a code in the tree can have. return value is error.*/
static upng_error huffman_tree_create_lengths(huffman_tree *tree, const uint16_t *bitlen)
{
    uint16_t *tree1d = (uint16_t*)app_malloc(sizeof(uint16_t) * MAX_SYMBOLS);
    uint16_t blcount[MAX_BIT_LENGTH];
    uint16_t nextcode[MAX_BIT_LENGTH];
    if (!tree1d)
        return UPNG_ENOMEM;

    uint16_t bits, n, i;
    uint16_t nodefilled = 0; /*up to which node it is filled */
    uint16_t treepos = 0;    /*position in the tree (1 of the numcodes columns) */

    /* initialize local vectors */
    memset(blcount, 0, sizeof(uint16_t) * MAX_BIT_LENGTH);
    memset(nextcode, 0, sizeof(uint16_t) * MAX_BIT_LENGTH);

    /*step 1: count number of instances of each code length */
    for (bits = 0; bits < tree->numcodes; bits++)
    {
        blcount[bitlen[bits]]++;
    }

    /*step 2: generate the nextcode values */
    for (bits = 1; bits <= tree->maxbitlen; bits++)
    {
        nextcode[bits] = (nextcode[bits - 1] + blcount[bits - 1]) << 1;
    }

    /*step 3: generate all the codes */
    for (n = 0; n < tree->numcodes; n++)
    {
        if (bitlen[n] != 0)
        {
            tree1d[n] = nextcode[bitlen[n]]++;
        }
    }

    /*convert tree1d[] to tree2d[][]. In the 2D array, a value of 32767 means uninited, a value >= numcodes is an address to another bit, a value < numcodes is a code. The 2 rows are the 2 possible bit values (0 or 1), there are as many columns as codes - 1
        a good huffmann tree has N * 2 - 1 nodes, of which N - 1 are internal nodes. Here, the internal nodes are stored (what their 0 and 1 option point to). There is only memory for such good tree currently, if there are more nodes (due to too long length codes), error 55 will happen */
    for (n = 0; n < tree->numcodes * 2; n++)
    {
        tree->tree2d[n] = 32767; /*32767 here means the tree2d isn't filled there yet */
    }

    for (n = 0; n < tree->numcodes; n++)
    { /*the codes */
        for (i = 0; i < bitlen[n]; i++)
        { /*the bits for this code */
            unsigned char bit = (unsigned char)((tree1d[n] >> (bitlen[n] - i - 1)) & 1);
            /* check if oversubscribed */
            if (treepos > tree->numcodes - 2)
                return UPNG_EMALFORMED;

            if (tree->tree2d[2 * treepos + bit] == 32767)
            { /*not yet filled in */
                if (i + 1 == bitlen[n])
                {                                        /*last bit */
                    tree->tree2d[2 * treepos + bit] = n; /*put the current code in it */
                    treepos = 0;
                }
                else
                { /*put address of the next step in here, first that address has to be found of course (it's just nodefilled + 1)... */
                    nodefilled++;
                    tree->tree2d[2 * treepos + bit] = nodefilled + tree->numcodes; /*addresses encoded with numcodes added to it */
                    treepos = nodefilled;
                }
            }
            else
            {
                treepos = tree->tree2d[2 * treepos + bit] - tree->numcodes;
            }
        }
    }

    for (n = 0; n < tree->numcodes * 2; n++)
    {
        if (tree->tree2d[n] == 32767)
        {
            tree->tree2d[n] = 0; /*remove possible remaining 32767's */
        }
    }
    app_free(tree1d);
    return UPNG_EOK;
}

static uint16_t huffman_decode_symbol(const unsigned char *in, unsigned long *bp, const huffman_tree *codetree, unsigned long inlength)
{
    uint16_t treepos = 0, ct;
    unsigned char bit;
    for (;;)
    {
        /* error: end of input memory reached without endcode */
        if (((*bp) & 0x07) == 0 && ((*bp) >> 3) > inlength)
            return INVALID_CODE_INDEX;

        bit = read_bit(bp, in);

        ct = codetree->tree2d[(treepos << 1) | bit];
        if (ct < codetree->numcodes)
        {
            return ct;
        }

        treepos = ct - codetree->numcodes;
        if (treepos >= codetree->numcodes)
            return INVALID_CODE_INDEX;
    }
}

/* get the tree of a deflated block with dynamic tree, the tree itself is also Huffman compressed with a known tree*/
static upng_error get_tree_inflate_dynamic(huffman_tree *codetree, huffman_tree *codetreeD, huffman_tree *codelengthcodetree, const unsigned char *in, unsigned long *bp, unsigned long inlength)
{
    uint16_t codelengthcode[NUM_CODE_LENGTH_CODES];
    uint16_t *bitlen = (uint16_t *)app_malloc(sizeof(uint16_t) * NUM_DEFLATE_CODE_SYMBOLS);
    uint16_t bitlenD[NUM_DISTANCE_SYMBOLS];

    if (!bitlen)
        return UPNG_ENOMEM;

    uint16_t n, hlit, hdist, hclen, i;

    /*make sure that length values that aren't filled in will be 0, or a wrong tree will be generated */
    /*C-code note: use no "return" between ctor and dtor of an uivector! */
    if ((*bp) >> 3 >= inlength - 2)
        return UPNG_EMALFORMED;

    /* clear bitlen arrays */
    memset(bitlen, 0, sizeof(uint16_t) * NUM_DEFLATE_CODE_SYMBOLS);
    memset(bitlenD, 0, sizeof(uint16_t) * NUM_DISTANCE_SYMBOLS);

    /*the bit pointer is or will go past the memory */
    hlit = read_bits(bp, in, 5) + 257; /*number of literal/length codes + 257. Unlike the spec, the value 257 is added to it here already */
    hdist = read_bits(bp, in, 5) + 1;  /*number of distance codes. Unlike the spec, the value 1 is added to it here already */
    hclen = read_bits(bp, in, 4) + 4;  /*number of code length codes. Unlike the spec, the value 4 is added to it here already */

    for (i = 0; i < NUM_CODE_LENGTH_CODES; i++)
    {
        if (i < hclen)
        {
            codelengthcode[CLCL[i]] = read_bits(bp, in, 3);
        }
        else
        {
            codelengthcode[CLCL[i]] = 0; /*if not, it must stay 0 */
        }
    }

    upng_error error = huffman_tree_create_lengths(codelengthcodetree, codelengthcode);

    /* bail now if we encountered an error earlier */
    if (error != UPNG_EOK)
        return error;

    /*now we can use this tree to read the lengths for the tree that this function will return */
    i = 0;
    while (i < hlit + hdist)
    { /*i is the current symbol we're reading in the part that contains the code lengths of lit/len codes and dist codes */
        uint16_t code = huffman_decode_symbol(in, bp, codelengthcodetree, inlength);
        if (code == INVALID_CODE_INDEX)
            return UPNG_EMALFORMED;

        if (code <= 15)
        { /*a length code */
            if (i < hlit)
            {
                bitlen[i] = code;
            }
            else
            {
                bitlenD[i - hlit] = code;
            }
            i++;
        }
        else if (code == 16)
        {                           /*repeat previous */
            uint16_t replength = 3; /*read in the 2 bits that indicate repeat length (3-6) */
            uint16_t value;         /*set value to the previous code */

            if ((*bp) >> 3 >= inlength)
                return UPNG_EMALFORMED;
            /*error, bit pointer jumps past memory */
            replength += read_bits(bp, in, 2);

            if ((i - 1) < hlit)
            {
                value = bitlen[i - 1];
            }
            else
            {
                value = bitlenD[i - hlit - 1];
            }

            /*repeat this value in the next lengths */
            for (n = 0; n < replength; n++)
            {
                /* i is larger than the amount of codes */
                if (i >= hlit + hdist)
                    return UPNG_EMALFORMED;

                if (i < hlit)
                {
                    bitlen[i] = value;
                }
                else
                {
                    bitlenD[i - hlit] = value;
                }
                i++;
            }
        }
        else if (code == 17)
        {                           /*repeat "0" 3-10 times */
            uint16_t replength = 3; /*read in the bits that indicate repeat length */
            if ((*bp) >> 3 >= inlength)
                return UPNG_EMALFORMED;

            /*error, bit pointer jumps past memory */
            replength += read_bits(bp, in, 3);

            /*repeat this value in the next lengths */
            for (n = 0; n < replength; n++)
            {
                /* error: i is larger than the amount of codes */
                if (i >= hlit + hdist)
                    return UPNG_EMALFORMED;

                if (i < hlit)
                {
                    bitlen[i] = 0;
                }
                else
                {
                    bitlenD[i - hlit] = 0;
                }
                i++;
            }
        }
        else if (code == 18)
        {                            /*repeat "0" 11-138 times */
            uint16_t replength = 11; /*read in the bits that indicate repeat length */
            /* error, bit pointer jumps past memory */
            if ((*bp) >> 3 >= inlength)
                return UPNG_EMALFORMED;

            replength += read_bits(bp, in, 7);

            /*repeat this value in the next lengths */
            for (n = 0; n < replength; n++)
            {
                /* i is larger than the amount of codes */
                if (i >= hlit + hdist)
                    return UPNG_EMALFORMED;
                if (i < hlit)
                    bitlen[i] = 0;
                else
                    bitlenD[i - hlit] = 0;
                i++;
            }
        }
        else /* somehow an unexisting code appeared. This can never happen. */
            return UPNG_EMALFORMED;
    }

    if (bitlen[256] == 0)
        return UPNG_EMALFORMED;

    /*the length of the end code 256 must be larger than 0 */
    /*now we've finally got hlit and hdist, so generate the code trees, and the function is done */
    error = huffman_tree_create_lengths(codetree, bitlen);
    if (error != UPNG_EOK)
        return error;
    error = huffman_tree_create_lengths(codetreeD, bitlenD);
    if (error != UPNG_EOK)
        return error;
    app_free(bitlen);
    return UPNG_EOK;
}

/*inflate a block with dynamic of fixed Huffman tree*/
static upng_error inflate_huffman(unsigned char *out, unsigned long outsize, const unsigned char *in, unsigned long *bp, unsigned long *pos, unsigned long inlength, uint16_t btype)
{
    // Converted to malloc, was overflowing 2k stack on Pebble
    uint16_t *codetree_buffer = (uint16_t *)app_malloc(sizeof(uint16_t) * DEFLATE_CODE_BUFFER_SIZE);
    uint16_t codetreeD_buffer[DISTANCE_BUFFER_SIZE];
    if (codetree_buffer == NULL)
        return UPNG_EMALFORMED;
    uint16_t done = 0;

    huffman_tree codetree;
    huffman_tree codetreeD;

    if (btype == 1)
    {
        /* fixed trees */
        huffman_tree_init(&codetree, (uint16_t *)FIXED_DEFLATE_CODE_TREE, NUM_DEFLATE_CODE_SYMBOLS, DEFLATE_CODE_BITLEN);
        huffman_tree_init(&codetreeD, (uint16_t *)FIXED_DISTANCE_TREE, NUM_DISTANCE_SYMBOLS, DISTANCE_BITLEN);
    }
    else if (btype == 2)
    {
        /* dynamic trees */
        uint16_t codelengthcodetree_buffer[CODE_LENGTH_BUFFER_SIZE];
        huffman_tree codelengthcodetree;

        huffman_tree_init(&codetree, codetree_buffer, NUM_DEFLATE_CODE_SYMBOLS, DEFLATE_CODE_BITLEN);

        huffman_tree_init(&codetreeD, codetreeD_buffer, NUM_DISTANCE_SYMBOLS, DISTANCE_BITLEN);
        huffman_tree_init(&codelengthcodetree, codelengthcodetree_buffer, NUM_CODE_LENGTH_CODES, CODE_LENGTH_BITLEN);

        get_tree_inflate_dynamic(&codetree, &codetreeD, &codelengthcodetree, in, bp, inlength);
    }

    while (done == 0)
    {
        uint16_t code = huffman_decode_symbol(in, bp, &codetree, inlength);
        if (code == INVALID_CODE_INDEX)
            return UPNG_EMALFORMED;

        if (code == 256)
        {
            /* end code */
            done = 1;
        }
        else if (code <= 255)
        {
            /* literal symbol */
            if ((*pos) >= outsize)
                return UPNG_EMALFORMED;

            /* store output */
            out[(*pos)++] = (unsigned char)(code);
        }
        else if (code >= FIRST_LENGTH_CODE_INDEX && code <= LAST_LENGTH_CODE_INDEX)
        { /*length code */
            /* part 1: get length base */
            unsigned long length = LENGTH_BASE[code - FIRST_LENGTH_CODE_INDEX];
            uint16_t codeD, distance, numextrabitsD;
            unsigned long start, forward, backward, numextrabits;

            /* part 2: get extra bits and add the value of that to length */
            numextrabits = LENGTH_EXTRA[code - FIRST_LENGTH_CODE_INDEX];

            /* error, bit pointer will jump past memory */
            if (((*bp) >> 3) >= inlength)
                return UPNG_EMALFORMED;
            length += read_bits(bp, in, numextrabits);

            /*part 3: get distance code */
            codeD = huffman_decode_symbol(in, bp, &codetreeD, inlength);
            if (codeD == INVALID_CODE_INDEX)
                return UPNG_EMALFORMED;

            /* invalid distance code (30-31 are never used) */
            if (codeD > 29)
                return UPNG_EMALFORMED;

            distance = DISTANCE_BASE[codeD];

            /*part 4: get extra bits from distance */
            numextrabitsD = DISTANCE_EXTRA[codeD];

            /* error, bit pointer will jump past memory */
            if (((*bp) >> 3) >= inlength)
                return UPNG_EMALFORMED;

            distance += read_bits(bp, in, numextrabitsD);

            /*part 5: fill in all the out[n] values based on the length and dist */
            start = (*pos);
            backward = start - distance;

            if ((*pos) + length > outsize)
                return UPNG_EMALFORMED;

            for (forward = 0; forward < length; forward++)
            {
                out[(*pos)++] = out[backward];
                backward++;

                if (backward >= start)
                {
                    backward = start - distance;
                }
            }
        }
    }

    app_free(codetree_buffer);
    return UPNG_EOK;
}

static upng_error inflate_uncompressed(unsigned char *out, unsigned long outsize, const unsigned char *in, unsigned long *bp, unsigned long *pos, unsigned long inlength)
{
    unsigned long p;
    uint16_t len, nlen, n;

    /* go to first boundary of byte */
    while (((*bp) & 0x7) != 0)
    {
        (*bp)++;
    }
    p = (*bp) / 8; /*byte position */

    /* read len (2 bytes) and nlen (2 bytes) */
    if (p >= inlength - 4)
        return UPNG_EMALFORMED;

    len = in[p] + 256 * in[p + 1];
    p += 2;
    nlen = in[p] + 256 * in[p + 1];
    p += 2;

    /* check if 16-bit nlen is really the one's complement of len */
    if (len + nlen != 65535)
        return UPNG_EMALFORMED;

    if ((*pos) + len >= outsize)
        return UPNG_EMALFORMED;

    /* read the literal data: len bytes are now stored in the out buffer */
    if (p + len > inlength)
        return UPNG_EMALFORMED;

    for (n = 0; n < len; n++)
    {
        out[(*pos)++] = in[p++];
    }

    (*bp) = p * 8;
    return UPNG_EOK;
}

/*inflate the deflated data (cfr. deflate spec); return value is the error*/
static upng_error uz_inflate_data(unsigned char *out, unsigned long outsize, const unsigned char *in, unsigned long insize, unsigned long inpos)
{
    unsigned long bp = 0;  /*bit pointer in the "in" data, current byte is bp >> 3, current bit is bp & 0x7 (from lsb to msb of the byte) */
    unsigned long pos = 0; /*byte position in the out buffer */

    uint16_t done = 0;

    while (done == 0)
    {
        uint16_t btype;

        /* ensure next bit doesn't point past the end of the buffer */
        if ((bp >> 3) >= insize)
            return UPNG_EMALFORMED;

        /* read block control bits */
        done = read_bit(&bp, &in[inpos]);
        btype = read_bit(&bp, &in[inpos]) | (read_bit(&bp, &in[inpos]) << 1);

        /* process control type appropriateyly */
        upng_error error;
        if (btype == 3)
            error = UPNG_EMALFORMED;
        else if (btype == 0)
            error = inflate_uncompressed(out, outsize, &in[inpos], &bp, &pos, insize); /*no compression */
        else
            error = inflate_huffman(out, outsize, &in[inpos], &bp, &pos, insize, btype); /*compression, btype 01 or 10 */

        /* stop if an error has occured */
        if (error != UPNG_EOK)
            return error;
    }

    return UPNG_EOK;
}

extern upng_error uz_inflate(unsigned char *out, unsigned long outsize, const unsigned char *in, unsigned long insize)
{
    /* we require two bytes for the zlib data header */
    if (insize < 2)
        return UPNG_EMALFORMED;

    /* 256 * in[0] + in[1] must be a multiple of 31, the FCHECK value is supposed to be made that way */
    if ((in[0] * 256 + in[1]) % 31 != 0)
        return UPNG_EMALFORMED;

    /*error: only compression method 8: inflate with sliding window of 32k is supported by the PNG spec */
    if ((in[0] & 15) != 8 || ((in[0] >> 4) & 15) > 7)
        return UPNG_EMALFORMED;

    /* the specification of PNG says about the zlib stream: "The additional flags shall not specify a preset dictionary." */
    if (((in[1] >> 5) & 1) != 0)
        return UPNG_EMALFORMED;

    /* create output buffer */
    uz_inflate_data(out, outsize, in, insize, 2);

    return UPNG_EOK;
}
