#include "plfs_xtc.h"
#include <stdlib.h>
#include <string.h>

#define MAXATOMS 65408

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


int pp_all_bytescnt(int *st, int *leng)
{
	FILE *fp;
	int bytecnt, i, *ptr;
	char *c;
	fp = fopen("H:/Data-for-VMD/water-channel/step7_extend-400ns-fit-390ns-allwater.xtc", "rb");
	if (fp == NULL)
	{
		printf("fopen error\n");
		return 0;
	}
	fseek(fp, 88, SEEK_CUR);

	c = (char *)&i;
	ptr = st;
	*leng = 0;
	while(fread(c, 4, 1, fp) == 1)
	{
		bytecnt = (int)(xdr_ntohl (i));
		*ptr++ = bytecnt;
		++(*leng);
		// printf("%10d\n", bytecnt);
		if(bytecnt % 4)
		{
			bytecnt += (4 - (bytecnt % 4));
		}
		fseek(fp, bytecnt, SEEK_CUR);
		fseek(fp, 88, SEEK_CUR);
	}
	fclose(fp);
	return 0;
}


int get_bitsize(int *bytelist, int *bitlist, int *bitslist, \
                int *bytelen,  int *bitlen,  int *bitslen)
{
	FILE *fp;
	float precision;
	int size, minint[3], maxint[3], smallidx, bytecnt;
	fp = fopen("H:/Data-for-VMD/water-channel/step7_extend-400ns-fit-390ns-allwater.xtc", "rb");
	if (fp == NULL)
	{
		printf("fopen error\n");
		return 0;
	}

	int *byteptr, *bitprt, *bitsptr;
	byteptr  = bytelist;
	bitprt   = bitlist;
	bitsptr  = bitslist;
	*bytelen = 0;
	*bitlen  = 0;
	*bitslen = 0;
	while(1)
	{
		if(fseek(fp, 52, SEEK_CUR) == -1) return 0;
	
		if(fread((char*)&size, 4, 1, fp) != 1) return 0;
		size = (int)(xdr_ntohl (size));
	
		if(fread((char*)&precision, 4, 1, fp) != 1) return 0;
		int tmp = (unsigned int)(xdr_ntohl (precision));
		memcpy(&precision, &tmp, 4);
	
		if(fread((char*)&minint[0], 4, 1, fp) != 1) return 0;
		minint[0] = (int)(xdr_ntohl (minint[0]));
		if(fread((char*)&minint[1], 4, 1, fp) != 1) return 0;
		minint[1] = (int)(xdr_ntohl (minint[1]));
		if(fread((char*)&minint[2], 4, 1, fp) != 1) return 0;
		minint[2] = (int)(xdr_ntohl (minint[2]));
	
		if(fread((char*)&maxint[0], 4, 1, fp) != 1) return 0;
		maxint[0] = (int)(xdr_ntohl (maxint[0]));
		if(fread((char*)&maxint[1], 4, 1, fp) != 1) return 0;
		maxint[1] = (int)(xdr_ntohl (maxint[1]));
		if(fread((char*)&maxint[2], 4, 1, fp) != 1) return 0;
		maxint[2] = (int)(xdr_ntohl (maxint[2]));
	
		if(fread((char*)&smallidx, 4, 1, fp) != 1) return 0;
		smallidx = (int)(xdr_ntohl (smallidx));
	
		if(fread((char*)&bytecnt, 4, 1, fp) != 1) return 0;
		bytecnt = (int)(xdr_ntohl (bytecnt));
	
		*bytelist++ = bytecnt;
		++(*bytelen);

		if (bytecnt % 4)
			bytecnt = bytecnt + (4 - (bytecnt % 4));
		if(fseek(fp, bytecnt, SEEK_CUR)) return 0;
	
		// printf("%d\n", size);
		// printf("%f\n", precision);
		// printf("%d  %d  %d\n", minint[0], minint[1], minint[2]);
		// printf("%d  %d  %d\n", maxint[0], maxint[1], maxint[2]);
		// printf("%d\n", smallidx);
		// printf("%d\n", bytecnt);
	
		int diffint[3];
		diffint[0] = maxint[0] - minint[0]+1;
		diffint[1] = maxint[1] - minint[1]+1;
		diffint[2] = maxint[2] - minint[2]+1;
		int bitsize = 0;
		int bitsizeint[3];
		if ((diffint[0] | diffint[1] | diffint[2] ) > 0xffffff) {
			bitsizeint[0] = xtc_sizeofint(diffint[0]);
			bitsizeint[1] = xtc_sizeofint(diffint[1]);
			bitsizeint[2] = xtc_sizeofint(diffint[2]);
			bitsize = 0; /* flag the use of large sizes */
		} else {
			bitsize = xtc_sizeofints(3, diffint);
		}
	
		if(bitsize == 0)
		{
			*bitsptr++ = bitsizeint[0];
			*bitsptr++ = bitsizeint[1];
			*bitsptr++ = bitsizeint[2];
			++(*bitslen);
		}else
		{
			*bitprt++ = bitsize;
			++(*bitlen);
		}
	}

	fclose(fp);
	return 0;
}

int main()
{
	int bytelist[5000], bitlist[5000], bitslist[5000], bytelen, bitlen, bitslen;
	get_bitsize(bytelist, bitlist, bitslist, &bytelen, &bitlen, &bitslen);
	for(int i = 0; i < bytelen; ++i)
	{
		printf("%10d  %10d\n", bytelist[i], bitlist[i]);
	}
	printf("%d %d %d\n", bytelen, bitlen, bitslen);
	return 0;


	int st[5000], leng;
	memset(st, 0, 5000);
	pp_all_bytescnt(st, &leng);
	int max = INT_MIN, min = INT_MAX, avg = 0, sum = 0;
	// printf("%d\n", leng);
	for(int i = 0; i < leng; ++i)
	{
		if(max < st[i]) max = st[i];
		if(min > st[i]) min = st[i];
		sum += st[i];
	}
	avg = sum/(leng);
	printf("%d  %d  %d\n", max, min, avg);
	return 0;

	md_file *mf, *mf_write, *mf_read;
	md_header mdh;
	md_ts mt;

	// test read frame 
	printf("----> test read frmae <----\n");
	mf = xtc_open("../../water-channel/step7_extend-400ns-fit-390ns-allwater.xtc", MDFMT_XTC, XTC_READ);
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
	printf("\n\n\n\n----> test write frmae <----\n");
	mf_write = xtc_open("./zzz.xtc", MDFMT_XTC, XTC_WRITE);
	if(xtc_write_frame(mf, &mt))
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