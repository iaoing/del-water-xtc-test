#include "plfs_xtc.h"
#include <stdlib.h>
#include <string.h>

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
	// for(int i = 0; i < 20; ++i)
	// {
	// 	printf("index: %d ----> %f %f %f\n", i+1, mt->pos[i*3], mt->pos[i*3+1], mt->pos[i*3+2]);
	// }
}

int main()
{
	md_file *mf, *mf_write, *mf_read;
	md_header mdh;
	md_ts mt;

	// open file
	// mf = xtc_open("H:/Data-for-VMD/water-channel/step7_extend-400ns-fit-390ns-allwater.xtc", MDFMT_XTC, XTC_READ);
	// mf_write = xtc_open("./test1.xtc", MDFMT_XTC, XTC_WRITE);

	// test read header
	// if(xtc_read_header(mf, &mdh))
	// {
	// 	printf("%s\n", mdio_errmsg(mdio_errno()));
	// 	exit(1);
	// }
	// print_mdh(&mdh);

	// test read frame 
	printf("----> test read frmae <----\n");
	mf = xtc_open("H:/Data-for-VMD/Villin/CLONE6/frame0.xtc", MDFMT_XTC, XTC_READ);
	if(xtc_read_frame(mf, &mt))
	{
		printf("%s\n", mdio_errmsg(mdio_errno()));
		exit(1);
	}
	print_mdts(&mt);


	if(xtc_read_frame(mf, &mt))
	{
		printf("%s\n", mdio_errmsg(mdio_errno()));
		exit(1);
	}
	print_mdts(&mt);

	if(xtc_read_frame(mf, &mt))
	{
		printf("%s\n", mdio_errmsg(mdio_errno()));
		exit(1);
	}
	print_mdts(&mt);

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


	printf("----> test read frmae <----\n");
	mf = xtc_open("H:/Data-for-VMD/Villin/CLONE6/frame1.xtc", MDFMT_XTC, XTC_READ);
	if(xtc_read_frame(mf, &mt))
	{
		printf("%s\n", mdio_errmsg(mdio_errno()));
		exit(1);
	}
	print_mdts(&mt);


	if(xtc_read_frame(mf, &mt))
	{
		printf("%s\n", mdio_errmsg(mdio_errno()));
		exit(1);
	}
	print_mdts(&mt);

	if(xtc_read_frame(mf, &mt))
	{
		printf("%s\n", mdio_errmsg(mdio_errno()));
		exit(1);
	}
	print_mdts(&mt);

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

	//test write frame
	// modify data in mt
	// for(int i = mt.natoms/2; i < mt.natoms; ++i)
	// {
	// 	mt.pos[i*3 + 0] = 0;
	// 	mt.pos[i*3 + 1] = 0;
	// 	mt.pos[i*3 + 2] = 0;
	// }
	// printf("\n\n\n\n----> test write frmae <----\n");
	// mf_write = xtc_open("./test1.xtc", MDFMT_XTC, XTC_WRITE);
	// if(xtc_write_frame(mf, &mt))
	// {
	// 	printf("%s\n", mdio_errmsg(mdio_errno()));
	// 	exit(1);
	// }
	// if(xtc_close(mf_write))
	// {
	// 	printf("%s\n", mdio_errmsg(mdio_errno()));
	// 	exit(1);
	// }

	// // test reRead frame
	// printf("\n\n\n\n----> test reRead frmae <----\n");
	// mf_read = xtc_open("./test1.xtc", MDFMT_XTC, XTC_READ);
	// if(xtc_read_frame(mf_read, &mt))
	// {
	// 	printf("%s\n", mdio_errmsg(mdio_errno()));
	// 	exit(1);
	// }
	// print_mdts(&mt);
	// if(xtc_close(mf_read))
	// {
	// 	printf("%s\n", mdio_errmsg(mdio_errno()));
	// 	exit(1);
	// }

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