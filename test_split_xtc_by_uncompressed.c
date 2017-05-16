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
	while(!xtc_read_frame(mf, &mt))
	{
		if(count >= 10)
			break;
		++count;
		// modify write frame
		printf("========================== modify write frame ===================\n");
		// set 0 by uncompressed
		float *pos_ptr = mt.pos;
		int len;
		for (int i = 0; i < water_cnt/2; ++i)
		{
			pos_ptr += (water_index[i*2] * 3);
			len = (water_index[i*2+1] - water_index[i*2] + 1) * 3;
			memset((char*)pos_ptr, 0, len * sizeof(*pos_ptr));
		}


		// write frame
		printf("========================== test write frame ===================\n");
		if(xtc_write_frame(mf_write, &mt))
		{
			printf("%s\n", mdio_errmsg(mdio_errno()));
			exit(1);
		}

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