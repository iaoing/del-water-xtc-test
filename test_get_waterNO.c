#include "plfs_pdb.h"
#include "plfs_xtc.h"

#include <string.h>
#include <stdlib.h>

#define xtcName "H:/Data-for-VMD/water-channel/step7_extend-400ns-fit-390ns-allwater.xtc"
#define pdbName "H:/Data-for-VMD/water-channel/apo63run3-ex3-2769000.pdb"

int get_bytecnt(int *byte_index, int *byte_length);
int get_real_water_no(int *wn_index, int *wn_length);

int get_bytecnt(int *byte_index, int *byte_length)
{
	FILE *fp;
	int bytecnt, i, *ptr;
	char *c;
	fp = fopen(xtcName, "rb");
	if (fp == NULL)
	{
		printf("fopen error\n");
		return 0;
	}
	fseek(fp, 88, SEEK_CUR);

	c = (char *)&i;
	ptr = byte_index;
	*byte_length = 0;
	while(fread(c, 4, 1, fp) == 1)
	{
		bytecnt = (int)(xdr_ntohl (i));
		*ptr++ = bytecnt;
		++(*byte_length);
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

int main(void)
{
	int natoms, watoms, *water_index, water_length;
	if(get_pdb_natoms(pdbName, &natoms, &watoms, &water_index, &water_length))
	{
		printf("%s\n", __LINE__);
	}


	int byte_index[5000], byte_length;
	memset(byte_index, 0, 5000);
	get_bytecnt(byte_index, &byte_length);


	// printf("natoms: %d\n", natoms);
	// printf("water_length: %d\n", water_length);
	// printf("water_index: %d  %d\n", water_index[water_length-2], water_index[water_length-1]);
	// printf("byte_length: %d\n", byte_length);
	// printf("byte_index: %d  %d\n", byte_index[byte_length-2], byte_index[byte_length-1]);

	int all_wn_index[byte_length][water_length];
	// all_wn_index = (int**)malloc(byte_length * sizeof(int*));

	// printf("%ld  %ld  %ld\n", &all_wn_index[0][0], &all_wn_index[1][0], &all_wn_index[2][0]);
	int break_no = 0;
	FILE *fp;
	fp = fopen(xtcName, "rb");
	for(int i = 0; i , byte_length; ++i)
	{
		if(break_no > 10)  break;
		++break_no;
		// int *wn_index;
		// wn_index = (int*)malloc((water_length) * sizeof(int));
		if(xtc_get_water_no(fp, water_index, water_length, &all_wn_index[i][0]) != 1)
		{
			printf("nnnnnnn\n");
		}
		// memcpy((char*)&all_wn_index[i][0], (char*)wn_index, water_length * sizeof(int));
		// free(wn_index);
	}

	for(int i = 0; i < water_length; ++i)
	{
		printf("water_index: %d\n", water_index[i]);
	}

	for(int i = 0; i < 10; ++i)
	{
		printf("byte_index: %d\n", byte_index[i]);
		printf("all_wn_index: ");
		for(int j = 0; j < water_length; ++j)
		{
			printf("%10d", all_wn_index[i][j]);
		}
		printf("\n");
	}

	fclose(fp);

	return 0;
}