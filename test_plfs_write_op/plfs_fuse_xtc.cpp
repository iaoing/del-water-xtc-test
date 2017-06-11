// #include <errno.h>
#include <string>
#include <fstream>
#include <iostream>
#include <fcntl.h>
#include <iostream>
#include <limits>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
// #include <sys/types.h>
// #include <sys/dir.h>
// #include <sys/param.h>
// #include <sys/mount.h>
// #include <sys/statvfs.h>
// #include <sys/time.h>
// #include <time.h>
// #include <pwd.h>
// #include <grp.h>
#include <map>
// #include <iomanip>
#include <iostream>
// #include <fstream>
// #include <sstream>

#include "plfs_fuse_xtc.h"


using namespace std;


//////////////////////////////////////////////////////////////////
/// reorganize xtc file
//////////////////////////////////////////////////////////////////

bool Pxtc::isPdbFile(string fname)
{
    int len;
    len = fname.length();
    if(fname[len-4] == '.' && fname[len-3] == 'p' && fname[len-2] == 'd' && fname[len-1] == 'b'){
        return 1;
    }
    return 0;
}

int Pxtc::addPdbFile(string path)
{
    map< string, pdb_info >::iterator iter;
    iter = self->pdb_files.find(path);
    if(iter == self->pdb_files.end())
    {
        pdb_info pi;
        pi.natoms = -1;
        self->pdb_files[path] = pi;
    }
    return 0;
}

bool Pxtc::isXtcFile(string fname)
{
    int len;
    len = fname.length();
    if(fname[len-4] == '.' && fname[len-3] == 'x' && fname[len-2] == 't' && fname[len-1] == 'c'){
        return 1;
    }
    return 0;
}

int Pxtc::addPdbInfo(string path)
{
    map< string, vector<string> >::iterator xpmiter;
    xpmiter = self->xtc_pdb_map.find(path);
    if(xpmiter == self->xtc_pdb_map.end())
    {
        // get all pdb file path in this dir;
        vector<string> bestPdb = getBestPdb(path);
        if (bestPdb.size() > 0){
            // establish this map;
            self->xtc_pdb_map[path] = bestPdb;
            // then filling pdb info if not filled;
            for(int i = 0; i < bestPdb.size(); ++i)
            {
                map< string, pdb_info >::iterator pfiter;
                pfiter = self->pdb_files.find(bestPdb[i]);
                if(pfiter == self->pdb_files.end()){
                    // case could happend after restart plfs server;
                    self->pdb_files[bestPdb[i]] = getPdbInfo(bestPdb[i]);
                }else if(pfiter->second.natoms == -1){
                    // means not filled pdb info;
                    self->pdb_files[bestPdb[i]] = getPdbInfo(bestPdb[i]);
                }else{
                    // means pdb info already in it's position;
                    ;
                }
            }
        }
    }
    return 0;
}

int Pxtc::addXtcFile(string path)
{
    map< string, xtc_info >::iterator iter;
    iter = self->xtc_files.find(path);
    if(iter == self->xtc_files.end())
    {
        xtc_info xi;
        xi.natoms = -1;     // means not filling this info;
        xi.frame_cnt = 0;
        self->xtc_files[path] = xi;
    }
    return 0;
}

vector<string> Pxtc::getBestPdb(string expended)
{
    map< string, pdb_info >::iterator iter;
    vector<string> pdbpaths;
    string dest, src;
    dest = skipLastName(expended);
    for(iter = self->pdb_files.begin(); iter != self->pdb_files.end(); ++iter)
    {
        src = skipLastName(iter->first);
        if(dest == src){
            pdbpaths.push_back(iter->first);
        }
    }
    return pdbpaths;
}


pdb_info Pxtc::getPdbInfo(string path)
{
    int type, serialNO, natoms, sLen;
    char buf[100];
    vector<int>::iterator iter;
    pdb_info pdbi;
    FILE *fp;

    fp = fopen(path.c_str(), "r");
    if(fp == NULL){
        return pdbi;
    }

    iter = pdbi.waterNO.begin();
    sLen = 0;
    while(fgets(buf, 99, fp))
    {
        type = get_atom_type(buf, &serialNO);
        if(type == PDB_ATOM) ++natoms;
        if(type == PDB_WATER)
        {
            ++natoms;
            if(sLen == 0){
                pdbi.waterNO.push_back(serialNO);
                ++sLen;
            }else if(sLen % 2 != 0){
                if(*(iter+sLen-1) + 1 == serialNO){
                    pdbi.waterNO.push_back(serialNO);
                    ++sLen;
                }else{
                    pdbi.waterNO.push_back(*(iter+sLen-1));
                    pdbi.waterNO.push_back(serialNO);
                    sLen += 2;
                }
            }else{
                if(*(iter+sLen-1) + 1 == serialNO){
                    *(iter+sLen-1) = serialNO;
                }else{
                    pdbi.waterNO.push_back(*(iter+sLen-1));
                    ++sLen;
                }
            }
        }
    }

    fclose(fp);

    if(sLen % 2 != 0){
        pdbi.waterNO.push_back(*(iter+sLen-1));
        ++sLen;
    }


    pdbi.natoms = natoms;
    return pdbi;
}


// getWaterSerialNO(string expendedPdbPath, vector<int> waterNO)
// {
//     vector<string> fname;
//     fname = findPdbPath(expendedPdbPath.c_str());
// }


// vector<string> findPdbPath(const char *expanded)
// {
//     int ret = PLFS_SUCCESS;
//     struct plfs_physpathinfo ppi;
//     const char *stripped_path;
//     DIR *dir;
//     struct dirent *ptr;
//     vector<string> all_name, pdb_name;
//     int len;

//     stripped_path = skipPrefixPath(expanded);
//     ret = plfs_resolvepath(stripped_path, &ppi);

//     if(ret)
//     {
//         // get dir path;
//         const char *dirPath = skipLastName(ppi->canbpath);
//         if(!isDirectory(dirPath, ppi->canback->store)){
//             return pdb_name;
//         }
//         // read dir;
//         dir = opendir(dirPath);
//         while((ptr = readdir(dir)) != NULL){
//             all_name.push_back(string(ptr->d_name));
//         }
//         closedir(dir);
//     }

//     // get .pdb filename
//     for(string name : all_name){
//         len = name.length();
//         if(name[len-3] == 'p' && name[len-2] == 'd' && name[len-1] == 'b'){
//             // is .pdb file;
//             // earse element form vector is expensive, so restore it in another vector
//             pdb_name.push_back(name);
//         }
//     }


//     return pdb_name;

// }

string Pxtc::skipLastName(string expended)
{
    int npos;
    npos = expended.rfind("/");
    if(npos <= 0){
        return "/";
    }
    return expended.substr(0, npos);
}

int Pxtc::get_atom_type(char *str, int *serialNO)
{
    *serialNO = 0;
    if(strlen(str) < 30 || str[0] != 'A' || str[1] != 'T' || str[2] != 'O' || str[3] != 'M')
    {
        return PDB_MARK;
    }

    char serial[5], resName[4];
    memcpy(serial, str+6, 5);
    memcpy(resName, str+17, 4);
    if(resName[0] != 'T' || resName[1] != 'I' || resName[2] != 'P' || resName[3] != '3')
    {
        return PDB_ATOM;        // not water atom
    }
    *serialNO = atoi(serial);
    return PDB_WATER;
}


int Pxtc::hold_buf(string expended, const char *buf, size_t size, off_t offset)
{
    int still_hold = 0;
    map< string, xtc_buffer >::iterator hb_iter;
    hb_iter = self->hold_buffer.find(expended);
    if(hb_iter == self->hold_buffer.end())
    {
        xtc_buffer xbuf;
        xbuf.trunc_flag = 0;
        xbuf.frame_cnt  = 0;
        xbuf.offset     = offset;
        xbuf.size       = size;
        xbuf.cf_bytes   = 0;
        xbuf.pi_ptr     = NULL;
        xbuf.xi_ptr     = NULL;

        xbuf.buffer.resize(size);
        memcpy((char*)&(xbuf.buffer[0]), buf, size);
        // xbuf.buffer     = string(buf);

        self->hold_buffer.insert(pair<string, xtc_buffer>(expended, xbuf));
        // self->hold_buffer[expended] = xbuf;
        hb_iter = self->hold_buffer.find(expended);
    }else{
        if(hb_iter->second.trunc_flag != 1)
        {
            if(hb_iter->second.size == 0)
                return -1;  // can't trunc, and not should flush buffer, so go on in p_write;
            else
                return -3;  // means should flush buffer before turn to p_write;
        }
        // !!NOTE!! should check buffer_offset + buffer_size == offset!
        // !!NOTE!! but it seems many conditions should be considered!
        hb_iter->second.buffer.resize(hb_iter->second.size + size);
        memcpy((char*)&(hb_iter->second.buffer[hb_iter->second.size]), buf, size);
        hb_iter->second.size      += size;
        // hb_iter->second.buffer    += string(buf);
    }

    // if size >= HEADER_SIZE means can get header info and bytecnt from this buffer;
    if(hb_iter->second.size >= HEADER_SIZE)
    {
        if(hb_iter->second.cf_bytes == 0)
        {
            // means not get cf_bytes before, so get it;
            int bcnt;
            get_bcnt_from_buf(&(hb_iter->second.buffer[0]), &bcnt);
            assert(bcnt);
            if(bcnt % 4)
            {
                bcnt += (4 - bcnt % 4);
            }
            hb_iter->second.cf_bytes = bcnt;
        } // != 0 means get bytecnt already;
    }else{
        return 0;   // not write, still hold it;
    }

    // find the best pdb info from xtc_pdb_map before get xtc info and waterNO;
    // pi_ptr == NULL means not find the best pdb_info;
    // set pdb_info *pi_ptr;
    if(hb_iter->second.pi_ptr == NULL && hb_iter->second.trunc_flag == 0)
    {
        // first get natoms of this xtc file from header info;
        int xtc_natoms = 0;
        if(xtc_get_natoms(&(hb_iter->second.buffer), &xtc_natoms))
            xtc_natoms = 0;

        map< string, vector<string> >::iterator xpmap_iter;
        xpmap_iter = self->xtc_pdb_map.find(expended);
        if(xpmap_iter == self->xtc_pdb_map.end())
        {
            addPdbInfo(expended);
            xpmap_iter = self->xtc_pdb_map.find(expended);
        }
        if(xpmap_iter->second.size() >= 1)
        {
            vector<string>::iterator vs_ptr;
            int pdb_count = 0;
            for(vs_ptr = xpmap_iter->second.begin(); vs_ptr != xpmap_iter->second.end(); ++vs_ptr)
            {
                map< string, pdb_info >::iterator pfs_ptr;
                pfs_ptr = self->pdb_files.find(*vs_ptr);
                if(pfs_ptr == self->pdb_files.end()){
                    // WTF!!!
                    hb_iter->second.trunc_flag = -1;
                }else if(pfs_ptr->second.natoms == xtc_natoms){
                    ++pdb_count;
                    hb_iter->second.trunc_flag = 1;
                    hb_iter->second.pi_ptr = &(pfs_ptr->second);
                }
            }
            if(pdb_count != 1){ // can't determain only pdb_info;
                hb_iter->second.trunc_flag = -1;
            }
            if(pdb_count > 1){  // more than one pdb_info targeted;
                hb_iter->second.trunc_flag = -2;
            }
        }else{
            hb_iter->second.trunc_flag = -1;
        }
    }

    if(hb_iter->second.trunc_flag != 1)
        return -2;  // should flush buffer before return directry;

    if(hb_iter->second.xi_ptr == NULL)
    {
        map< string, xtc_info >::iterator xfs_iter;
        xfs_iter = self->xtc_files.find(expended);
        if(xfs_iter == self->xtc_files.end())
        {
            addXtcFile(expended);
        }
        xfs_iter = self->xtc_files.find(expended);
        hb_iter->second.xi_ptr = &(xfs_iter->second);
    }


    // means current frame is matured, then can write it;
    if(hb_iter->second.size >= HEADER_SIZE + hb_iter->second.cf_bytes)
    {
        if(hb_iter->second.frame_cnt == 0)
        {
            // if current frame is the first frame, we should to get base info to fill xtc_info;
            // set xtc_info *xi_ptr
            int *c, *wn_index;
            c = (int*)malloc(2 * sizeof(int));
            wn_index = (int*)malloc(2 * sizeof(int));
            get_max_water_no(&(hb_iter->second.pi_ptr->waterNO), c);

            xtc_get_water_no(&(hb_iter->second.buffer), c, 2, wn_index);

            if(hb_iter->second.xi_ptr == NULL)
            {
                // WTF!!
                hb_iter->second.trunc_flag = -3;
                return -2;
            }

            hb_iter->second.xi_ptr->best_water_begin = *c;
            hb_iter->second.xi_ptr->best_water_end = *(++c);
            hb_iter->second.xi_ptr->base_best_begin = *wn_index;
            hb_iter->second.xi_ptr->base_best_end = *(++wn_index);
            hb_iter->second.xi_ptr->first_bytecnt = hb_iter->second.cf_bytes;
            hb_iter->second.xi_ptr->natoms = hb_iter->second.pi_ptr->natoms;
            hb_iter->second.xi_ptr->frame_cnt = 0;
            free(c);
            free(wn_index);

            // write this first frame to drive wholly;
            // bcz we should get xtc_info again if restart server from first frame;
            // so keep the first frame integrated;
            return 1;
        }else{
            // should calc offset of the water atomes in current frame;
            // and truncate it; then write it;
            return 2;
        }
    }else{
        // means current frame is immature, still hold this buffer;
        return 0;
    }
    return 0;
}


int Pxtc::get_bcnt_from_buf(const char *buf, int *i)
{
    return get_int(buf + HEADER_SIZE - 4, i);
}


int Pxtc::get_int(const char *buf, int *i)
{
    int mycopy;
    unsigned char *c;
    c = (unsigned char*)&mycopy;
    memcpy(c, buf, 4);
  
    *i = (int)(xdr_ntohl (mycopy));
    return 0;
}

int Pxtc::get_float(const char *buf, float *f)
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
unsigned int Pxtc::xdr_ntohl(unsigned int x)
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

// xdr_htonl -> real int to (char*)int for write
// unsigned int xdr_htonl(unsigned int x)
// {
//     short s = 0x0F00;
//     if (*((char *)&s) == (char)0x0F)
//     {
//         /* bigendian, do nothing */
//         return x;
//     }
//     else
//     {
//         /* smallendian,swap bytes */
//         return xdr_swapbytes(x);
//     }
// }


unsigned int Pxtc::xdr_swapbytes(unsigned int x)
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



//////////////////////////////////////////////////////
// other tools to get more info

int Pxtc::xtc_get_natoms(string *hold_buf, int *natoms)
{
    char *buf_begin, *buf_ptr;
    buf_begin = (char*)&((*hold_buf)[0]);
    buf_ptr = buf_begin + 4;

    if (get_int(buf_ptr, natoms) < 0) 
    {
        return -1;
    }
    if(*natoms >0 && *natoms <100000)
        return 0;
    return -1;
}

int Pxtc::xtc_get_water_no(string *hold_buf, int *water_index, int water_length, int *wn_index)
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
        return -1;
    }
    buf_ptr += 4;

    // if (*size != 0 && lsize != *size) return mdio_seterror(MDIO_BADFORMAT);
    *size = lsize;
    fp = (float*)malloc((lsize * 3) * sizeof(float));

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

// *d must malloc 2 * sizeof(int)
int Pxtc::get_max_water_no(vector<int> *src, int *d)
{
    if(src->size() == 2)
    {
        *d = (*src)[0];
        *(++d) = (*src)[1];
        return 0;
    }

    if(src->size() < 2)
        return -1;

    if(src->size() % 2)
        src->push_back((*src)[src->size()-1]);


    int max = 0, sub = 0;
    for(int i = 0; i < src->size(); i += 2)
    {
        if((*src)[i+1] - (*src)[i] > max)
        {
            max = (*src)[i+1] - (*src)[i];
            sub = i;
        }
    }

    *d = (*src)[sub];
    *(++d) = (*src)[sub+1];
    return 0;
}



///////////////////////////////////////////////////////////////////////
// This algorithm is an implementation of the 3dfcoord algorithm
// written by Frans van Hoesel (hoesel@chem.rug.nl) as part of the
// Europort project in 1995.
///////////////////////////////////////////////////////////////////////

// returns the number of bits in the binary expansion of
// the given integer.
int Pxtc::xtc_sizeofint(int size) {
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
int Pxtc::xtc_sizeofints(int nints, unsigned int *sizes) {
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
int Pxtc::xtc_receivebits(int *buf, int nbits) {
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
void Pxtc::xtc_receiveints(int *buf, const int nints, int nbits,
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


int Pxtc::xtc_flush_buffer(string expended, size_t realsize)
{
    int ret, bytes_written;
    map< string, xtc_buffer >::iterator hb_iter;
    int err;

    hb_iter = self->hold_buffer.find(expended);
    if(hb_iter == self->hold_buffer.end())
    {
        err = PLFS_TBD;
        return -99;
    }
    FILE *fp = fopen(expended.c_str(), "rb");
    fseek(fp, 0, SEEK_END);
    err = fwrite(hb_iter->second.buffer.c_str(), hb_iter->second.size, 1, fp );
    fclose(fp);

    if(err == 1)
    {
        string tmp;
        hb_iter->second.buffer.swap(tmp);
        hb_iter->second.size = 0;
        hb_iter->second.offset += hb_iter->second.size;
    }

    ret = (err == 1) ? realsize : -99;
    return ret;
}


int Pxtc::xtc_flush_frame(string expended, size_t realsize)
{
    int ret, bytes_written;
    map< string, xtc_buffer >::iterator hb_iter;
    int err;

    hb_iter = self->hold_buffer.find(expended);
    if(hb_iter == self->hold_buffer.end())
    {
        // WTF!!!
        err = -99;
        return -99;
    }

    if(hb_iter->second.size < hb_iter->second.cf_bytes + HEADER_SIZE)
    {
        // WTF!!!
        err = -99;
        return -99;
    }

    size_t size;
    off_t  offset;
    char* buf;
    size = hb_iter->second.cf_bytes + HEADER_SIZE;
    offset = hb_iter->second.offset;
    buf = (char*)malloc(size + 2);
    memcpy(buf, (char*)&(hb_iter->second.buffer[0]), size);
    buf[size+1] = '\0';

    FILE *fp = fopen(expended.c_str(), "rb");
    fseek(fp, 0, SEEK_END);
    err = fwrite( buf, size, 1, fp );
    fclose(fp);

    if(err == 1)    
    {
        hb_iter->second.xi_ptr->frame_cnt = (++(hb_iter->second.frame_cnt));
        hb_iter->second.size -= size;
        hb_iter->second.offset += size;
        hb_iter->second.buffer = hb_iter->second.buffer.erase(0, size);
    }


    ret = (err == 1) ? realsize : -99;
    return ret;
}


int Pxtc::xtc_trunc_flush_frame(string expended, size_t realsize)
{
    int ret, bytes_written;
    map< string, xtc_buffer >::iterator hb_iter;
    int err;

    hb_iter = self->hold_buffer.find(expended);
    if(hb_iter == self->hold_buffer.end())
    {
        // WTF!!!
        err = PLFS_TBD;
        return -99;
    }

    if(hb_iter->second.size < hb_iter->second.cf_bytes + HEADER_SIZE)
    {
        // WTF!!!
        err = PLFS_TBD;
        return -99;
    }

    size_t size;
    off_t  offset;
    int buf_size;
    char *buf;
    size = hb_iter->second.cf_bytes + HEADER_SIZE;
    offset = hb_iter->second.offset;
    buf_size = size;
    ret = xtc_truncate_frame(&buf, &buf_size, &(hb_iter->second));
    if(ret)
    {
        err = PLFS_TBD;
        return -99;
    }
    // buf = (char*)malloc(size + 2);
    // memcpy(buf, (char*)&(hb_iter->second.buffer[0]), size);
    // buf[size+1] = '\0';

    FILE *fp;
    fseek(fp, 0, SEEK_END);
    err = fwrite( buf, buf_size, 1, fp);
    fclose(fp);


    if(err == 1)    
    {
        hb_iter->second.xi_ptr->frame_cnt = (++(hb_iter->second.frame_cnt));
        hb_iter->second.size -= size;
        hb_iter->second.offset += size;
        hb_iter->second.buffer = hb_iter->second.buffer.erase(0, size);
    }


    ret = (err == 1) ? realsize : -99;
    return ret;
}

// frame: ppppppppwwwwwwwppppppp (Protein, Water)
// -----> pppppppp       ppppppp
// -----> ppppppppppppppp
int Pxtc::xtc_truncate_frame(char **buf, int *size, xtc_buffer *xb_iter)
{
    int delta_begin, delta_end, delta_bytes;
    float base  = 0.005;
    delta_bytes = (xb_iter->cf_bytes - xb_iter->xi_ptr->first_bytecnt);
    delta_begin = xb_iter->xi_ptr->best_water_begin * base;
    delta_end   = xb_iter->xi_ptr->best_water_end * base;

    int sp_begin, sp_end;
    sp_begin = xb_iter->xi_ptr->base_best_begin + delta_bytes + delta_begin;
    sp_end   = xb_iter->xi_ptr->base_best_end   + delta_bytes - delta_begin - delta_end;

    // cp & erase(sp_begin, sp_end);
    int ret, d_size, s_size = *size;
    ret = split_buf(buf, (char*)&(xb_iter->buffer[0]), sp_begin, sp_end, &d_size, s_size);
    if (ret)
        return -1;
    *size = d_size;
    return 0;
}


int Pxtc::split_buf(char **buf, char *src, int sp_begin, int sp_end, int *d_size, int s_size)
{
    int ret = 0;
    *d_size = sp_begin + (s_size - sp_end);
    *buf = (char*)malloc(*d_size * sizeof(char) + 10);
    if(*buf == NULL)
        return -1;
    memcpy(*buf, src, sp_begin);
    memcpy((*buf + sp_begin + 1), (src + sp_end), (s_size - sp_end));
    // *buf[d_size] = '\0';
    return 0;
}




// for test
int Pxtc::write_file(const char *strPath, const char *buf, size_t size, off_t offset)
{
    int ret;
    if(isXtcFile(strPath))
    {
        map< string, xtc_info >::iterator xfs_iter;
        xfs_iter = self->xtc_files.find(strPath);
        if(xfs_iter == self->xtc_files.end())
        {
            addXtcFile(strPath);
        }
        // map< string, vector<string> >::iterator xpmap_iter;
        // xpmap_iter = self->xtc_pdb_map.find(strPath);
        // if(xpmap_iter = self->xtc_pdb_map.end())
        // {
        //     addPdbInfo(strPath);
        // }
        // hold the buf until get the whole header, 
        // then handle it to get header info.
        int xtc_ret;
        xtc_ret = hold_buf(strPath, buf, size, offset);
        if(xtc_ret == 0){
            // hold buffer, not write;
            ret = size;
        }else if(xtc_ret == -3){
            // flush buffer, then goto f_write, exec current writing;
            ret = xtc_flush_buffer(strPath, size);
        }else if(xtc_ret == -2){
            // flush buffer, then return directly;
            ret = xtc_flush_buffer(strPath, size);
        }else if(xtc_ret == -1){
            // not flush buffer, goto f_write, exec current writing;
            ret = size;
        }else if(xtc_ret == 1){
            // first frame not other cause, flush frame wholely;
            // hold buffer, not write;
            ret = xtc_flush_frame(strPath, size);
        }else if(xtc_ret == 2){
            // split current frame and flush it;
            // hold buffer, not write;
            ret = xtc_trunc_flush_frame(strPath, size);
        }
        return xtc_ret;
    }else{
        return -99;
    }
    return -100;
}


