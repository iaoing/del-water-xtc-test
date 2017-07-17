#include <set>
#include <string>
#include <map>
#include <vector>
#include <list>
#include <unistd.h>

using namespace std;



// frame struct: header + bytecnt(int) + compressed Coord(whose length = bytecnt)
// header struct: magicNUM(int) + natoms(int) + strp(int) + timev(float) + box[3][3](float)
//                + size(== natoms int) + precision(float) + minint[3](int) + maxint[3](int)
//                + smallinx(int)
#define HEADER_SIZE     92      // contain header and bytecnt;

struct pdb_info
{
    int natoms;
    vector<int> waterNO;
};

// WRITE:
// * first frame:
//  hold buf until reach HEADER_SIZE, then get header info, then write header buf and hold other buf;
//  get bytecnt from buf that holded;
//  hold compressed coord buf until reach bytecnt, then decompressed it and get base water info, write it at last;
// * next frame:
//  hold buf until read HEADER_SIZE, then write header;
//  get bytecnt from remined buf;
//  hold compressed coord buf until reach bytecnt, then trunc it accordance with base water info, write it at last;
// READ:
// * first frame:
// * next frame:
struct xtc_info
{
    // int read_cnt;
    // int write_cnt;
    int natoms;
    int frame_cnt;                  // number of frames already wr
    // int cur_bycnt;               // bytecnt of cur frame(compressed coord bytes)
    // int cur_offset;              // offset of cur frame(offset of compressdd coord)
    // int pre_offset;              // total offset of all pre frames
    // int total_size;              // size that already wr, not necessary, bcz plfs using pwrite & pread so we know this offset;
    int first_bytecnt;
    int base_best_begin;            // base offset of water in frame
    int base_best_end;              // base offset of water in frame
    // vector<int> base_offset;     // not using;
    int best_water_begin;
    int best_water_end;
};

struct xtc_buffer
{
    int rw_flag;
    int trunc_flag;         // flag that indicate should trunc xtc file or not;
                            // 0 means not get pdb info before;
                            // -1 means natoms of pdb info is not equal to natoms of xtc file;
                            // -2 means too many pdb info equal to xtc info and can't distinguish;
                            // -3 means WTF!!;
                            // 1 means got it!
    int frame_cnt;          // number of frame that had writed;
    off_t offset;           // begining offset that should write to;
    size_t size;            // size of this hold buffer;
    int cf_bytes;           // current frame bytecnt that indicate the length of the compressed coord buf;
                            // if cf_bytes == 0 means not hold enough buf to get header info include bytecnt;
                            // else means we can write buffer until HEADER_SIZE + cf_bytes <= buffer.size();
                            // after write should reset cf_bytes = 0;
    pdb_info *pi_ptr;
    xtc_info *xi_ptr;
    string buffer;          // buffer which to hold buf until got a whole frame;
                            // should remove water info when could write it;
};


struct xtc_read_buf
{
    int trunc_flag;
    int frame_cnt;

    off_t offset;       // at the view of the user;
    off_t f_offset;     // for reading the truncated file;
    size_t size;
    int cf_bytes;

    pdb_info *pi_ptr;
    xtc_info *xi_ptr;
    string buffer;
};

// integer table used in decompression
static int xtc_magicints[] = {
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

// type of line in .pdb file
#define PDB_MARK    0       // means this line is a mark
#define PDB_ATOM    1       // means this line is an atom
#define PDB_WATER   2       // means this line is a water atom

#define PLFS_SUCCESS 0

#define TAG_SIZE        (7*sizeof(int))
#define PLFS_TBD        -99


class Pxtc 
{
    public:
        static Pxtc *self;
        init_self(Pxtc *t){
            self = t;
            pthread_rwlock_init( &(write_lock), NULL );
        }
        //////////////////////////////////////////////////
        /// reorganize xtc plfs
        static bool isPdbFile(string);
        static int addPdbFile(string);
        static bool isXtcFile(string);
        static int addPdbInfo(string);
        static int addXtcFile(string);
        static vector<string> getBestPdb(string);
        static pdb_info getPdbInfo(string);
        // static int getWaterSerialNo(string, vector<int>);
        // static vector<string> findPdbPath(string expaneded);
        // static int checkNAtoms(const char *, const char *);
        static string skipLastName(string);
        static int get_atom_type(char *, int *);
        static int hold_buf(string, const char *, size_t, off_t);
        static int get_bcnt_from_buf(const char *buf, int *i);

        // int                             trunc_xtc;     // flag that indicate should trunc xtc file or not;
        map< string, vector<string> >   xtc_pdb_map;   // xtc-expended-path -->> pdb-expended-path
        map< string, xtc_info >         xtc_files;     // xtc-expended-path -->> xtc-info  
        map< string, pdb_info >         pdb_files;     // pdb-expended-path -->> pdb-info
        map< string, xtc_buffer >       hold_buffer;   // xtc-expended-path -->> hold_buffer(write until hold a whole frame)
        map< string, xtc_read_buf >     read_buffer;


        pthread_rwlock_t            write_lock;

        ////////////////////////////////////////////////////
        /// some xtc tools
        static int get_int(const char *buf, int *i);
        static int get_float(const char *buf, float *f);
        // xdr_ntohl -> (*char)int to real int for read
        static unsigned int xdr_ntohl(unsigned int x);
        // xdr_htonl -> real int to (char*)int for write
        // static unsigned int xdr_htonl(unsigned int x);
        static unsigned int xdr_swapbytes(unsigned int x);

        static int xtc_sizeofint(int size);
        static int xtc_sizeofints(int nints, unsigned int *sizes);
        static int xtc_receivebits(int *buf, int nbits);
        static void xtc_receiveints(int *buf, const int nints, int nbits, unsigned int *sizes, int *nums);


        //////////////////////////////////////////////////////
        /// other tools to get more info
        static int xtc_get_natoms(string *hold_buf, int *natoms);
        static int xtc_get_water_no(string *hold_buf, int *water_index, int water_length, int *wn_index);
        static int get_max_water_no(vector<int> *src, int *d);

        // 
        static int xtc_flush_buffer(string expended, size_t realsize);
        static int xtc_flush_frame(string expended, size_t realsize);
        static int xtc_trunc_flush_frame(string expended, size_t realsize);

        //
        static int xtc_truncate_frame(char **buf, int *size, xtc_buffer *xb_iter);
        static int split_buf(char **dest, char *src, int sp_begin, int sp_end, int *d_size, int s_size);

        static int anaPdbBuf(char *buf, int buf_size, int *natoms, vector<int> *pp);
        static vector<string> findPdbPath(string expanded);



        ////////////////////////////////
        /// for read;
        static int read_xtc(const string expanded, char *buf, size_t size, off_t offset);
        static int read_add_xi(const string expanded);
        
        static int read_xtc_first_frame(const string expanded, xtc_read_buf *xrbuf);
        static int read_xtc_frame(const string expanded, xtc_read_buf *xrbuf);


        static int read_xtc_check(off_t offset, size_t size, const xtc_read_buf *xrbuf);

        static int read_xtc_cp_buf(xtc_read_buf *xrbuf, char *buf, size_t size, off_t offset);
        static int read_xtc_resize_buf(xtc_read_buf *xrbuf);



        // for close a file;
        static int xtc_close(const string strPath, int open_flag);
        static int xtc_write_close(string strPath);
        static int xtc_read_close(string strPath);





    public:
        // for test;
        static int write_file(const char *path, const char *buf, size_t size, off_t offset);
        static int read_file(const char *path, char *buf, size_t size, off_t offset);

        // for debug;
        static int p_write(int fd, const char *buf, size_t size, off_t offset);
        static int plfs_read(const string expanded, char *buf, size_t size, off_t offset, size_t *bytes_read);

        // for debug;
        static int bing_hold_buf(string expanded, const char *buf, size_t size, off_t offset);

};

