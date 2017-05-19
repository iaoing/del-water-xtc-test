#include "plfs_pdb.h"
#include "plfs_xtc.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define xtcName "H:/Data-for-VMD/water-channel/step7_extend-400ns-fit-390ns-allwater.xtc"
#define pdbName "H:/Data-for-VMD/water-channel/apo63run3-ex3-2769000.pdb"
#define writeName "H:/Data-for-VMD/water-channel/test_split.xtc"

#define headerLength  	88		// 88bytes + bytecnt + buf + 4bytesfixed
#define NATOMS 			45608

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
	printf("========================== END get water index ===================\n\n");

	// read frame 
	char *buf_header, *buf_coord;
	int bytecnt, breakcnt = 0;
	float bit_size;
	int mod_bit[water_cnt];

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

		printf("========================== modify frame ===================\n");
		// printf("BYTECNT: %d\n", bytecnt);
		bit_size = bytecnt * 8.0 / (NATOMS * 1.0);
		for(int i = 0; i < water_cnt/2; ++i)
		{
			int begin_bit = (water_index[i*2] - 1) * bit_size + 1;
			int bit_length = (water_index[i*2+1] - water_index[i*2] + 1) * bit_size - 1;
			mod_bit[i*2] = begin_bit;
			mod_bit[i*2+1] = bit_length;
		}
		for(int i = 0; i < water_cnt/2; ++i)
		{
			memset(buf_coord+((mod_bit[i*2]+7)/8), 0, (mod_bit[i*2+1])/8);
		}

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