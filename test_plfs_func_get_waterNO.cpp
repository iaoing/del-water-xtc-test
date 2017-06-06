#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

#define xtcName "H:/Data-for-VMD/water-channel/step7_extend-400ns-fit-390ns-allwater.xtc"

int xtc_magicints[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0,8, 10, 12, 16, 20, 25, 32, 40, 50, 64,
    80, 101, 128, 161, 203, 256, 322, 406, 512, 645, 812, 1024, 1290,
    1625, 2048, 2580, 3250, 4096, 5060, 6501, 8192, 10321, 13003, 16384,
    20642, 26007, 32768, 41285, 52015, 65536, 82570, 104031, 131072,
    165140, 208063, 262144, 330280, 416127, 524287, 660561, 832255,
    1048576, 1321122, 1664510, 2097152, 2642245, 3329021, 4194304,
    5284491, 6658042, 8388607, 10568983, 13316085, 16777216 };

#define FIRSTIDX 9
/* note that magicints[FIRSTIDX-1] == 0 */
#define LASTIDX (sizeof(xtc_magicints) / sizeof(*xtc_magicints))



string get_buf();
int xtc_get_water_no(string *hold_buf, int *water_index, int water_length, int *wn_index);
int get_int(const char *buf, int *i);
int get_float(const char *buf, float *f);
unsigned int xdr_ntohl(unsigned int x);
unsigned int xdr_swapbytes(unsigned int x);

int xtc_sizeofint(int size);
int xtc_sizeofints(int nints, unsigned int *sizes);
int xtc_receivebits(int *buf, int nbits);
void xtc_receiveints(int *buf, const int nints, int nbits, unsigned int *sizes, int *nums);


int get_int(const char *buf, int *i)
{
    int mycopy;
    unsigned char *c;
    c = (unsigned char*)&mycopy;
    memcpy(c, buf, 4);
  
    *i = (int)(xdr_ntohl (mycopy));
    return 0;
}

int get_float(const char *buf, float *f)
{
    int i, mycopy;
    unsigned char *c;
    c = (unsigned char*)&mycopy;
    memcpy(c, buf, 4);

    if(f)
    {
        i = (xdr_ntohl (mycopy));
        memcpy(f, &i, 4);
    }
    return 0;
}


// some tools for get for put data
// xdr_ntohl -> (*char)int to real int for read
unsigned int xdr_ntohl(unsigned int x)
{
    short s = 0x0F00;
    if (*((char *)&s) == (char)0x0F)
    {
        /* bigendian, do nothing */
        return x;
    }
    else
    {
        /* smallendian, swap bytes */
        return xdr_swapbytes(x);
    }
}


unsigned int xdr_swapbytes(unsigned int x)
{
    unsigned int y;
    int i;
    char        *px = (char *)&x;
    char        *py = (char *)&y;

    for (i = 0; i < 4; i++)
    {
        py[i] = px[3-i];
    }

    return y;
}


// returns the number of bits in the binary expansion of
// the given integer.
int xtc_sizeofint(int size) {
    unsigned int num = 1;
    unsigned int ssize = (unsigned int)size;
    int nbits = 0;

    while (ssize >= num && nbits < 32) {
        nbits++;
        num <<= 1;
    }
    return nbits;
}

// calculates the number of bits a set of integers, when compressed,
// will take up.
int xtc_sizeofints(int nints, unsigned int *sizes) {
    int i;
    unsigned int num;
    unsigned int nbytes, nbits, bytes[32], bytecnt, tmp;
    nbytes = 1;
    bytes[0] = 1;
    nbits = 0;
    for (i=0; i < nints; i++) { 
        tmp = 0;
        for (bytecnt = 0; bytecnt < nbytes; bytecnt++) {
            tmp = bytes[bytecnt] * sizes[i] + tmp;
            bytes[bytecnt] = tmp & 0xff;
            tmp >>= 8;
        }
        while (tmp != 0) {
            bytes[bytecnt++] = tmp & 0xff;
            tmp >>= 8;
        }
        nbytes = bytecnt;
    }
    num = 1;
    nbytes--;
    while (bytes[nbytes] >= num) {
        nbits++;
        num *= 2;
    }
    return nbits + nbytes * 8;
}

// reads bits from a buffer.    
int xtc_receivebits(int *buf, int nbits) {
    int cnt, num; 
    unsigned int lastbits, lastbyte;
    unsigned char * cbuf;
    int mask = (1 << nbits) -1;

    cbuf = ((unsigned char *)buf) + 3 * sizeof(*buf);
    cnt = buf[0];
    lastbits = (unsigned int) buf[1];
    lastbyte = (unsigned int) buf[2];

    num = 0;
    while (nbits >= 8) {
        lastbyte = ( lastbyte << 8 ) | cbuf[cnt++];
        num |=  (lastbyte >> lastbits) << (nbits - 8);
        nbits -=8;
    }
    if (nbits > 0) {
        if (lastbits < (unsigned int)nbits) {
            lastbits += 8;
            lastbyte = (lastbyte << 8) | cbuf[cnt++];
        }
        lastbits -= nbits;
        num |= (lastbyte >> lastbits) & ((1 << nbits) -1);
    }
    num &= mask;
    buf[0] = cnt;
    buf[1] = lastbits;
    buf[2] = lastbyte;
    return num; 
}

// decompresses small integers from the buffer
// sizes parameter has to be non-zero to prevent divide-by-zero
void xtc_receiveints(int *buf, const int nints, int nbits,
            unsigned int *sizes, int *nums) {
    int bytes[32];
    int i, j, nbytes, p, num;

    bytes[1] = bytes[2] = bytes[3] = 0;
    nbytes = 0;
    while (nbits > 8) {
        bytes[nbytes++] = xtc_receivebits(buf, 8);
        nbits -= 8;
    }
    if (nbits > 0) {
        bytes[nbytes++] = xtc_receivebits(buf, nbits);
    }
    for (i = nints-1; i > 0; i--) {
        num = 0;
        for (j = nbytes-1; j >=0; j--) {
            num = (num << 8) | bytes[j];
            p = num / sizes[i];
            bytes[j] = p;
            num = num - p * sizes[i];
        }
        nums[i] = num;
    }
    nums[0] = bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
}





string get_buf()
{
    string ret_string = "";
    FILE *fp;
    fp = fopen(xtcName, "rb");
    char *buf;
    buf = (char*)malloc(92 * sizeof(char));
    if(fread(buf, 92, 1, fp) != 1)
    {
        printf("fread error 1\n");
    }
    ret_string.resize(92);
    memcpy((char*)&ret_string[0], buf, 92);
    cout << "ret_string.size(): " << ret_string.size() << endl;
    int magic;
    get_int((char*)&(ret_string[0]), &magic);
    cout << "magic: " << magic << endl;

    char *buf_ptr;
    int bcnt;
    buf_ptr = buf;
    buf_ptr += 88;
    get_int(buf_ptr, &bcnt);
    printf("bcnt %d\n", bcnt);
    free(buf);


    buf = (char*)malloc(bcnt * sizeof(char));
    if(fread(buf, bcnt, 1, fp) != 1)
    {
        printf("fread error 2\n");
    }
    ret_string.resize(92 + bcnt);
    memcpy((char*)&ret_string[0] + 92, buf, bcnt);
    cout << "ret_string.size(): " << ret_string.size() << endl;

    free(buf);

    return ret_string;
}



int xtc_get_water_no(string *hold_buf, int *water_index, int water_length, int *wn_index)
{
    char *buf_begin, *buf_ptr;
    buf_begin = (char*)&((*hold_buf)[0]);
    buf_ptr = buf_begin + 52;

    int water_sub = 0, *wn_tmp_index;
    wn_tmp_index = (int*)malloc(water_length * sizeof(int));
    


    float *fp, *precision;
    int *size;
    size = (int*)malloc(sizeof(int));
    precision = (float*)malloc(sizeof(float));


    int *ip = NULL;
    int oldsize;
    int *buf;

    int minint[3], maxint[3], *lip;
    int smallidx;
    unsigned sizeint[3], sizesmall[3], bitsizeint[3], size3;
    int flag, k;
    int small, smaller, i, is_smaller, run;
    float *lfp;
    int tmp, *thiscoord,  prevcoord[3];

    int bufsize, lsize;
    unsigned int bitsize;
    float inv_precision;

    /* avoid uninitialized data compiler warnings */
    bitsizeint[0] = 0;
    bitsizeint[1] = 0;
    bitsizeint[2] = 0;


    if (get_int(buf_ptr, &lsize) < 0) 
    {
        // cout << __LINE__ << endl;
        return -1;
    }
    buf_ptr += 4;

    // if (*size != 0 && lsize != *size) return mdio_seterror(MDIO_BADFORMAT);
    *size = lsize;
    fp = (float*)malloc((lsize * 3) * sizeof(float));
    // cout << "        __LINE__: " << __LINE__ << endl;
    // *size = lsize;
    size3 = *size * 3;
    if (*size <= 9) {
        for (i = 0; i < *size; i++) {
            if (get_float(buf_ptr, fp + (3 * i)) < 0) return -1;
            buf_ptr += 4;
            if (get_float(buf_ptr, fp + (3 * i) + 1) < 0) return -1;
            buf_ptr += 4;
            if (get_float(buf_ptr, fp + (3 * i) + 2) < 0) return -1;
            buf_ptr += 4;
        }
        return *size;
    }

    // cout << "        __LINE__: " << __LINE__ << endl;

    get_float(buf_ptr, precision);
    buf_ptr += 4;
    if (ip == NULL) {
        ip = (int *)malloc(size3 * sizeof(*ip));
        if (ip == NULL) return -1;
        bufsize = (int) (size3 * 1.2);
        buf = (int *)malloc(bufsize * sizeof(*buf));
        if (buf == NULL) return -1;
        oldsize = *size;
    } else if (*size > oldsize) {
        ip = (int *)realloc(ip, size3 * sizeof(*ip));
        if (ip == NULL) return -1;
        bufsize = (int) (size3 * 1.2);
        buf = (int *)realloc(buf, bufsize * sizeof(*buf));
        if (buf == NULL) return -1;
        oldsize = *size;
    }
    buf[0] = buf[1] = buf[2] = 0;

    // minint[3]
    get_int(buf_ptr, &(minint[0]));
    buf_ptr += 4;
    get_int(buf_ptr, &(minint[1]));
    buf_ptr += 4;
    get_int(buf_ptr, &(minint[2]));
    buf_ptr += 4;
    // maxint[3]
    get_int(buf_ptr, &(maxint[0]));
    buf_ptr += 4;
    get_int(buf_ptr, &(maxint[1]));
    buf_ptr += 4;
    get_int(buf_ptr, &(maxint[2]));
    buf_ptr += 4;

        
    sizeint[0] = maxint[0] - minint[0]+1;
    sizeint[1] = maxint[1] - minint[1]+1;
    sizeint[2] = maxint[2] - minint[2]+1;
    
    /* check if one of the sizes is to big to be multiplied */
    if ((sizeint[0] | sizeint[1] | sizeint[2] ) > 0xffffff) {
        bitsizeint[0] = xtc_sizeofint(sizeint[0]);
        bitsizeint[1] = xtc_sizeofint(sizeint[1]);
        bitsizeint[2] = xtc_sizeofint(sizeint[2]);
        bitsize = 0; /* flag the use of large sizes */
    } else {
        bitsize = xtc_sizeofints(3, sizeint);
    }

    get_int(buf_ptr, &smallidx);
    buf_ptr += 4;
    smaller = xtc_magicints[FIRSTIDX > smallidx - 1 ? FIRSTIDX : smallidx - 1] / 2;
    small = xtc_magicints[smallidx] / 2;
    sizesmall[0] = sizesmall[1] = sizesmall[2] = xtc_magicints[smallidx];

    /* check for zero values that would yield corrupted data */
    if ( !sizesmall[0] || !sizesmall[1] || !sizesmall[2] ) {
        printf("XTC corrupted, sizesmall==0 (case 1)\n");
        return -1;
    }


    /* buf[0] holds the length in bytes */
    if (get_int(buf_ptr, &(buf[0])) < 0) 
    {
        return -1;
    }
    buf_ptr += 4;


    // copy buf(compressed coordinate) from buf_ptr to &buf[3] at length of buf[0];
    memcpy((char*)&buf[3], buf_ptr, (int)buf[0]);
    // if (xtc_data(mf, (char *) &buf[3], (int) buf[0]) < 0) 
    // {
    //     return -1;
    // }

    buf[0] = buf[1] = buf[2] = 0;

    lfp = fp;
    inv_precision = 1.0f / (*precision);
    run = 0;
    i = 0;
    lip = ip;
    water_sub = 0;
    while (i < lsize) {
        // bing...
        // if(water_sub < water_length && water_index[water_sub] == (i+1))
        // {
        //  wn_tmp_index[water_sub] = buf[0];
        //  ++water_sub;
        // }

        thiscoord = (int *)(lip) + i * 3;

        if (bitsize == 0) {
            thiscoord[0] = xtc_receivebits(buf, bitsizeint[0]);
            thiscoord[1] = xtc_receivebits(buf, bitsizeint[1]);
            thiscoord[2] = xtc_receivebits(buf, bitsizeint[2]);
        } else {
            xtc_receiveints(buf, 3, bitsize, sizeint, thiscoord);
        }

        i++;
        // bing...
        if(water_sub < water_length && water_index[water_sub] == (i+1))
        {
            wn_tmp_index[water_sub] = buf[0];
            ++water_sub;
        }

        thiscoord[0] += minint[0];
        thiscoord[1] += minint[1];
        thiscoord[2] += minint[2];

        prevcoord[0] = thiscoord[0];
        prevcoord[1] = thiscoord[1];
        prevcoord[2] = thiscoord[2];
 

        flag = xtc_receivebits(buf, 1);
        is_smaller = 0;
        if (flag == 1) {
            run = xtc_receivebits(buf, 5);
            is_smaller = run % 3;
            run -= is_smaller;
            is_smaller--;
        }
        if (run > 0) {
            thiscoord += 3;
            for (k = 0; k < run; k+=3) {
                xtc_receiveints(buf, 3, smallidx, sizesmall, thiscoord);
                i++;
                /*  by:bing 2017-06-06 */
                if(water_sub < water_length && water_index[water_sub] == (i+1))
                {
                    wn_tmp_index[water_sub] = buf[0];
                    ++water_sub;
                }/* END by:bing 2017-06-06 */

                thiscoord[0] += prevcoord[0] - small;
                thiscoord[1] += prevcoord[1] - small;
                thiscoord[2] += prevcoord[2] - small;
                if (k == 0) {
                    /* interchange first with second atom for better
                     * compression of water molecules
                     */
                    tmp = thiscoord[0]; thiscoord[0] = prevcoord[0];
                    prevcoord[0] = tmp;
                    tmp = thiscoord[1]; thiscoord[1] = prevcoord[1];
                    prevcoord[1] = tmp;
                    tmp = thiscoord[2]; thiscoord[2] = prevcoord[2];
                    prevcoord[2] = tmp;
                    *lfp++ = prevcoord[0] * inv_precision;
                    *lfp++ = prevcoord[1] * inv_precision;
                    *lfp++ = prevcoord[2] * inv_precision;

                    if ( !sizesmall[0] || !sizesmall[1] || !sizesmall[2] ) {
                        printf("XTC corrupted, sizesmall==0 (case 2)\n");
                        return -1;
                    }

                } else {
                    prevcoord[0] = thiscoord[0];
                    prevcoord[1] = thiscoord[1];
                    prevcoord[2] = thiscoord[2];
                }
                *lfp++ = thiscoord[0] * inv_precision;
                *lfp++ = thiscoord[1] * inv_precision;
                *lfp++ = thiscoord[2] * inv_precision;
            }
        } else {
            *lfp++ = thiscoord[0] * inv_precision;
            *lfp++ = thiscoord[1] * inv_precision;
            *lfp++ = thiscoord[2] * inv_precision;      
        }
        smallidx += is_smaller;
        if (is_smaller < 0) {
            small = smaller;
            if (smallidx > FIRSTIDX) {
                smaller = xtc_magicints[smallidx - 1] /2;
            } else {
                smaller = 0;
            }
        } else if (is_smaller > 0) {
            smaller = small;
            small = xtc_magicints[smallidx] / 2;
        }
        sizesmall[0] = sizesmall[1] = sizesmall[2] = xtc_magicints[smallidx] ;
    }

    // printf("water_sub: %d\n", water_sub);
    // *wn_index = (int*)malloc((water_length) * sizeof(int));
    memcpy((char*)wn_index, (char*)wn_tmp_index, water_length*sizeof(int));
    // printf("%d  %d\n", wn_index[0], wn_index[1]);
    
    free(wn_tmp_index);
    free(size);
    free(precision);
    free(fp);
    free(ip);
    free(buf);

    return 1;


}



int main()
{
    string hold_buf;
    hold_buf = get_buf();

    int *water_index;
    water_index = (int*)malloc(2 * sizeof(int));
    *(water_index) = 19297;
    *(water_index + 1) = 45561;
    int water_length = 2;
    int *wn_index;
    wn_index = (int*)malloc(2 * sizeof(int));
    printf("%d   %d\n", *water_index, *(water_index+1));

    if(xtc_get_water_no(&hold_buf, water_index, water_length, wn_index) < 0)
    {
        cout << "NNNNN" << endl;
    }
    cout << "YUYYYY" << endl;

    printf("%d   %d\n", *wn_index, *(wn_index+1));

    return 0;
}