#include "plfs_pdb.h"
#include "plfs_xtc.h"
#include <stdlib.h>
#include <string.h>

#define xtcName "../../water-channel/step7_extend-400ns-fit-390ns-allwater.xtc"
#define pdbName "../../water-channel/apo63run3-ex3-2769000.pdb"


void get_water_index(int **water_index, int *water_cnt)
{
	int natoms, watoms;
	if(get_pdb_natoms(pdbName, &natoms, &watoms, water_index, water_cnt))
	{
		pdb_dispaly_err();
	}

	pdb_dispaly_err();

}


int main()
{
	md_file *mf, *mf_write, *mf_read;
	md_header mdh;
	md_ts mt;

	mf = xtc_open(xtcName, MDFMT_XTC, XTC_READ);
	mf_write = xtc_open("./test1.xtc", MDFMT_XTC, XTC_WRITE);

	// get water index from .pdb
	printf("========================== get water index ===================\n");
	int *water_index, water_cnt;
	get_water_index(&water_index, &water_cnt);


	// read frame 
	int count = 0;
	char *buf;
	while(1)
	{
		if(count >= 10)
			break;
		++count;

		// read frame
		int magic, natoms, step;
		float timev, boxx[3], boxy[3], boxz[3];
		int size;
		float precision;
		int minint[3], maxint[3], smallidx, bytecnt;
		

		if(xtc_int(mf, &magic) < 0) break;
		if(xtc_int(mf, &natoms) < 0) break;
		if(xtc_int(mf, &step) < 0) break;

		if(xtc_float(mf, &timev) < 0) break;
		if(xtc_float(mf, &boxx[0]) < 0) break;
		if(xtc_float(mf, &boxx[1]) < 0) break;
		if(xtc_float(mf, &boxx[2]) < 0) break;
		if(xtc_float(mf, &boxy[0]) < 0) break;
		if(xtc_float(mf, &boxy[2]) < 0) break;
		if(xtc_float(mf, &boxy[3]) < 0) break;
		if(xtc_float(mf, &boxz[0]) < 0) break;
		if(xtc_float(mf, &boxz[1]) < 0) break;
		if(xtc_float(mf, &boxz[2]) < 0) break;

		if(xtc_int(mf, &size) < 0) break;

		if(xtc_float(mf, &precision) < 0) break;

		if(xtc_int(mf, &minint[0]) < 0) break;
		if(xtc_int(mf, &minint[1]) < 0) break;
		if(xtc_int(mf, &minint[2]) < 0) break;
		if(xtc_int(mf, &maxint[0]) < 0) break;
		if(xtc_int(mf, &maxint[1]) < 0) break;
		if(xtc_int(mf, &maxint[2]) < 0) break;
		if(xtc_int(mf, &smallidx) < 0) break;
		if(xtc_int(mf, &bytecnt) < 0) break;

		buf = (char*)malloc(bytecnt * sizeof(char));
		if(buf == NULL) printf("cnt: %d   buf == null, malloc error.\n", bytecnt);
		if(xtc_data(mf, buf, bytecnt) < 0) break;

		if(bytecnt % 4)
		{
			fseek(mf->f, 4 - (bytecnt % 4), SEEK_CUR);
		}

		// modify write frame and write it
		printf("========================== modify write frame ===================\n");
		// set 0 by not uncompressed
		if(xtc_int(mf_write, &magic) < 0) break;
		if(xtc_int(mf_write, &natoms) < 0) break;
		if(xtc_int(mf_write, &step) < 0) break;

		if(xtc_float(mf_write, &timev) < 0) break;
		if(xtc_float(mf_write, &boxx[0]) < 0) break;
		if(xtc_float(mf_write, &boxx[1]) < 0) break;
		if(xtc_float(mf_write, &boxx[2]) < 0) break;
		if(xtc_float(mf_write, &boxy[0]) < 0) break;
		if(xtc_float(mf_write, &boxy[2]) < 0) break;
		if(xtc_float(mf_write, &boxy[3]) < 0) break;
		if(xtc_float(mf_write, &boxz[0]) < 0) break;
		if(xtc_float(mf_write, &boxz[1]) < 0) break;
		if(xtc_float(mf_write, &boxz[2]) < 0) break;

		if(xtc_int(mf_write, &size) < 0) break;

		if(xtc_float(mf_write, &precision) < 0) break;

		if(xtc_int(mf_write, &minint[0]) < 0) break;
		if(xtc_int(mf_write, &minint[1]) < 0) break;
		if(xtc_int(mf_write, &minint[2]) < 0) break;
		if(xtc_int(mf_write, &maxint[0]) < 0) break;
		if(xtc_int(mf_write, &maxint[1]) < 0) break;
		if(xtc_int(mf_write, &maxint[2]) < 0) break;
		if(xtc_int(mf_write, &smallidx) < 0) break;
		if(xtc_int(mf_write, &bytecnt) < 0) break;

		int sizebyte_big, sizebyte_small, byte_beg, byte_end, byte_len;
		sizebyte_big = water_cnt/natoms/8 + 1;	// ceil
		sizebyte_small = water_cnt/natoms/8;					// floor
		char *buf_ptr;
		buf_ptr = buf;
		for (int i = 0; i < water_cnt/2; ++i)
		{
			byte_beg = water_index[i] * sizebyte_big;
			byte_end = (water_index[i+1] - water_index[i] + 1) * sizebyte_small;
			if(byte_beg >= byte_end) byte_len = sizebyte_small;
			memset(buf_ptr + byte_beg, 0, byte_len);
		}
		if(xtc_data(mf_write, buf, bytecnt) < 0) break;
		fflush(mf_write->f);

		free(buf);
		buf = NULL;


	}


	
	printf("%s\n", mdio_errmsg(mdio_errno()));
	

	
	

	
	if(xtc_close(mf))
	{
		printf("%s\n", mdio_errmsg(mdio_errno()));
		exit(1);
	}
	if(xtc_close(mf_write))
	{
		printf("%s\n", mdio_errmsg(mdio_errno()));
		exit(1);
	}
	
	return 0;
}