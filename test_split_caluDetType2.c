#include "plfs_pdb.h"
#include "plfs_xtc.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define xtcName "H:/Data-for-VMD/water-channel/step7_extend-400ns-fit-390ns-allwater.xtc"
#define pdbName "H:/Data-for-VMD/water-channel/apo63run3-ex3-2769000.pdb"
#define writeName "H:/Data-for-VMD/water-channel/test_split_caluDetType2.xtc"

#define headerLength  	88		// 88bytes + bytecnt + buf + 4bytesfixed
#define NATOMS 			45608
#define OVERHEAD		0.004

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
	md_file *mf_read, *mf_write;
	md_header mdh;
	md_ts mt;

	mf_read = xtc_open(xtcName, MDFMT_XTC, XTC_READ);
	mf_write = xtc_open(writeName, MDFMT_XTC, XTC_WRITE);

	// get water index from .pdb
	printf("========================== get water index ===================\n");
	int *water_index, water_cnt;
	get_water_index(&water_index, &water_cnt);

	for(int i = 0; i < water_cnt; ++i)
		printf("%d  \n", water_index[i]);

	// OverHead is:
	int OverHead[water_cnt];
	for(int i = 0; i < water_cnt; ++i)
	{
		OverHead[i] = OVERHEAD * water_index[i];
		printf("%d\n", OverHead[i]);
	}

	printf("========================== END get water index ===================\n\n");

	// get base byte for dealt
	int baseByte, baseAtomeByte[water_cnt];
	if(xtc_get_water_no(mf_read->f, water_index, water_cnt, baseAtomeByte) < 0)
	{
		printf("get water no error!\n");
		return 0;
	}
	for (int i = 0; i < water_cnt; ++i)
	{
		printf("%d   ", baseAtomeByte[i]);
	}
	printf("\n");


	// read frame 
	char *buf_header, *buf_coord;
	int bytecnt, breakcnt = 0;
	int mod_bit[water_cnt];
	int det_size = 0;

	// move to the head of the file steram
	rewind(mf_read->f);

	while(1)
	{
		if(breakcnt > 10) break;
		++breakcnt;

		printf("========================== read frame ===================\n");
		buf_header = (char *)malloc(headerLength * sizeof(char));
		if(buf_header == NULL)
		{
			printf("malloc error: %d\n", __LINE__);
			break;
		}
		if(fread(buf_header, headerLength, 1, mf_read->f) != 1)
		{
			printf("fread error: %d\n", __LINE__);
			break;
		}

		if(xtc_int(mf_read, &bytecnt) < 0){
			printf("xtc_int error: %d\n", __LINE__);
			break;
		}
		buf_coord = (char *)malloc(bytecnt * sizeof(char));
		if(buf_coord == NULL)
		{
			printf("malloc error: %d\n", __LINE__);
			break;
		}
		if(xtc_data(mf_read, buf_coord, bytecnt) < 0)
		{
			printf("xtc_data error: %d\n", __LINE__);
			break;
		}
		printf("BYTECNT: %d\n", bytecnt);
		if(breakcnt == 1) baseByte = bytecnt;
		// printf("baseByte: %d\n", baseByte);

		printf("========================== modify frame ===================\n");
		// printf("BYTECNT: %d\n", bytecnt);
		det_size = bytecnt - baseByte;		// det_size = bytecnt[i] - bytecnt[0]
		printf("det_size: %d\n", det_size);
		for(int i = 0; i < water_cnt/2; ++i)
		{
			int begin_byte = baseAtomeByte[i*2] + det_size + OverHead[i*2];
			int end_byte = baseAtomeByte[i*2+1] + det_size - OverHead[i*2+1];
			if(! ((begin_byte < bytecnt) && (begin_byte >=0) && (end_byte <= bytecnt)) ){
				begin_byte = 0;
				end_byte = 0;
			}
			mod_bit[i*2] = begin_byte;
			mod_bit[i*2+1] = end_byte;
		}
		for(int i = 0; i < water_cnt/2; ++i)
		{
			memset(buf_coord+(mod_bit[i*2]), 0, (mod_bit[i*2+1] - mod_bit[i*2]));
		}
		printf("%d  %d\n", mod_bit[0], mod_bit[1]);

		printf("========================== write frame ===================\n");
		if(fwrite(buf_header, headerLength, 1, mf_write->f) != 1)
		{
			printf("fwrite error: %d\n", __LINE__);
			break;
		}

		if(xtc_int(mf_write, &bytecnt) < 0){
			printf("xtc_int error: %d\n", __LINE__);
			break;
		}
		if(xtc_data(mf_write, buf_coord, bytecnt) < 0)
		{
			printf("xtc_data error: %d\n", __LINE__);
			break;
		}

		free(buf_header);
		free(buf_coord);
		buf_header = NULL;
		buf_coord = NULL;

		printf("\n\n\n");
	}


	printf("========================== END ===================\n\n");

		

	// 	// modify write frame and write it
	// 	printf("========================== modify write frame ===================\n");
	// 	// set 0 by not uncompressed
		
	// 	buf = NULL;


	// }


	
	printf("%s\n", mdio_errmsg(mdio_errno()));
	

	
	

	
	if(xtc_close(mf_read))
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