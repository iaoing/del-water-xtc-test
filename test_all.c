#include "plfs_xtc.h"
#include "plfs_pdb.h"
#include <stdlib.h>
#include <string.h>

#define xtcName "../../water-channel/step7_extend-400ns-fit-390ns-allwater.xtc"
#define pdbName "../../water-channel/apo63run3-ex3-2769000.pdb"

void print_mdh(md_header *mdh)
{
	printf("----> md_header: %d  %f\n", mdh->natoms, mdh->timeval);
}

void print_mdts(md_ts *mt)
{
	printf("----> natoms: %d  step: %d  time: %f  precision: %f\n", mt->natoms, mt->step, mt->time, mt->precision);
	printf("----> md_box:\nA: %f  B: %f  C: %f\nalpha: %f  beta: %f  gamma: %f\n",
		mt->box->A, mt->box->B, mt->box->C, 
		mt->box->alpha, mt->box->beta, mt->box->gamma );
	printf("----> pos:\n");
	for(int i = 0; i < 20; ++i)
	{
		printf("index: %d ----> %f %f %f\n", i+1, mt->pos[i*3], mt->pos[i*3+1], mt->pos[i*3+2]);
	}
}

void get_water_index(int **water_index, int *water_ptr)
{
	int natoms, watoms;
	if(get_pdb_natoms(pdbName, &natoms, &watoms, water_index, water_ptr))
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


	// test read frame 
	printf("----> test read frmae <----\n");
	mf = xtc_open(xtcName, MDFMT_XTC, XTC_READ);
	if(xtc_read_frame(mf, &mt))
	{
		printf("%s\n", mdio_errmsg(mdio_errno()));
		exit(1);
	}
	print_mdts(&mt);
	if(xtc_close(mf))
	{
		printf("%s\n", mdio_errmsg(mdio_errno()));
		exit(1);
	}


	// get water index
	printf("========================== get water index ===================\n");
	int *water_index, water_ptr;
	get_water_index(&water_index, &water_ptr);

	// modify write frame
	printf("========================== modify write frame ===================\n");
	float *pos_ptr = mt.pos;
	int len;
	for (int i = 0; i < water_ptr/2; ++i)
	{
		pos_ptr += (water_index[i*2] * 3);
		len = (water_index[i*2+1] - water_index[i*2] + 1) * 3;
		memset((char*)pos_ptr, 0, len * sizeof(*pos_ptr));
	}


	// test write frame
	printf("========================== test write frame ===================\n");
	mf_write = xtc_open("./test1.xtc", MDFMT_XTC, XTC_WRITE);
	if(xtc_write_frame(mf_write, &mt))
	{
		printf("%s\n", mdio_errmsg(mdio_errno()));
		exit(1);
	}
	if(xtc_close(mf_write))
	{
		printf("%s\n", mdio_errmsg(mdio_errno()));
		exit(1);
	}

	// test reRead frame
	printf("========================== test reRead frame ===================\n");
	printf("\n\n\n\n----> test reRead frmae <----\n");
	mf_read = xtc_open("./test1.xtc", MDFMT_XTC, XTC_READ);
	if(xtc_read_frame(mf_read, &mt))
	{
		printf("%s\n", mdio_errmsg(mdio_errno()));
		exit(1);
	}
	print_mdts(&mt);
	if(xtc_close(mf_read))
	{
		printf("%s\n", mdio_errmsg(mdio_errno()));
		exit(1);
	}

	// if(xtc_read_frame(mf, &mt))
	// {
	// 	printf("%s\n", mdio_errmsg(mdio_errno()));
	// 	exit(1);
	// }
	// print_mdts(&mt);

	// if(xtc_read_frame(mf, &mt))
	// {
	// 	printf("%s\n", mdio_errmsg(mdio_errno()));
	// 	exit(1);
	// }
	// print_mdts(&mt);


	// close file 
	
	
	return 0;
}