#include "plfs_pdb.h"
#include <stdlib.h>
#include <string.h>

// for debug
static int pdb_seterror(const char *errp)
{
	// printf("pdb_seterror!\n");
	char *cpy;
	cpy = (char*)malloc(strlen(errp) * sizeof(*cpy));
	memcpy(cpy, errp, strlen(errp));
	pdb_errbuf.ptr %= 20;
	pdb_errbuf.buf[pdb_errbuf.ptr++] = cpy;
	// printf("%d\n", pdb_errbuf.ptr);
}

int pdb_dispaly_err()
{
	// printf("pdb_dispaly_err!\n");
	// printf("%d\n", pdb_errbuf.ptr);
	if(pdb_errbuf.buf[0] == NULL)
		return PDB_SUCCESS;
	printf("IN PDB_DISPLAY_ERR:  ");

	int ptr = pdb_errbuf.ptr;
	--ptr;
	// not full
	if(pdb_errbuf.buf[ptr] == NULL)
	{
		for(int i = 0; i < ptr; ++i)
		{
			printf("%s\n", pdb_errbuf.buf[i]);
		}
	}else{	// full and cyc
		for(int i = 0; i < 20; ++i)
		{
			ptr %= 20;
			printf("%s\n", pdb_errbuf.buf[ptr++]);
		}
	}
	return PDB_SUCCESS;
}



// open and close a .pdb file
static FILE *pdb_open(const char *fn, const int mode)
{
	FILE *fp;

	if(!fn)
	{
		pdb_seterror("filename is null");
		return NULL;
	}

	int len = strlen(fn);
	if(fn[len-1] != 'b' || fn[len-2] != 'd' || fn[len-3] != 'p' || fn[len-4] != '.')
	{
		pdb_seterror("file name error, not .pdb file");
		return NULL;
	}

	// Finally, open the file
    if (mode)			// write mode
    {
    	pdb_seterror("not allowed write a .pdb file.");
    	return NULL;
    }
    else			// read mode
        fp = fopen(fn, "rb");

   	if(!fp)
   	{
   		pdb_seterror("open error, get file point is null");
   	}

   	return fp;
}


static int pdb_close(FILE *fp)
{
	if(!fp)
	{
		pdb_seterror("close error, parament file pointer is null");
		return PDB_BADPARAMS;
	}
	fclose(fp);
	return PDB_SUCCESS;
}


// get number of all atoms and number of water atomes
int get_pdb_natoms(const char *fn, int *natoms, int *nwaters, int **water_index, int *water_ptr)
{
	FILE *fp;
	int *index, type, serialNO, index_size = PDB_MAX_INDEX, index_ptr = 0;
	char buf[PDB_BUFFER_LENGTH];

	*natoms = 0;
	*nwaters = 0;
	index = (int*)malloc(PDB_MAX_INDEX * sizeof(*index));
	if(!index)
	{
		pdb_seterror("malloc error!");
		return -1;
	}
	fp = pdb_open(fn, PDB_READ);
	if(!fp)
	{
		pdb_seterror("open error");
		return -1;
	}

	while(fgets(buf, PDB_BUFFER_LENGTH, fp))
	{
		type = get_atom_type(buf, &serialNO);
		if(type == PDB_ATOM) ++(*natoms);
		if(type == PDB_WATER)
		{
			++(*natoms);
			++(*nwaters);
			if(index_ptr == 0)
			{
				// printf("1111\n");
				index[index_ptr++] = serialNO;
			}else if(index_ptr % 2 == 0)
			{
				// printf("22222\n");
				if(index[index_ptr-1] + 1 == serialNO)
					index[index_ptr-1] = serialNO;
				else
					index[index_ptr++] = serialNO;
			}else{
				// printf("33333\n");
				if(index[index_ptr-1] + 1 == serialNO)
					index[index_ptr++] = serialNO;
				else
				{
					int old = index[index_ptr - 1];
					index[index_ptr++] = old;
					index[index_ptr++] = serialNO;
				}
			}
		}
	}

	if (index_ptr % 2)
	{
		int old = index[index_ptr - 1];
		index[index_ptr++] = old;
	}

	// printf("%d\n", index_ptr);
	// for(int i = 0; i < index_ptr; ++i)
	// 	printf("%d  ", index[i]);
	// printf("\n");

	*water_ptr = index_ptr;
	*water_index = (int*)malloc((index_ptr) * sizeof(int));
	memcpy((char*)*water_index, (char*)index, index_ptr*sizeof(int));

	free(index);

	if(pdb_close(fp))
	{
		pdb_dispaly_err();
	}

	return PDB_SUCCESS;
}

static int get_atom_type(char *str, int *serialNO)
{
	if(strlen(str) < 30 || str[0] != 'A' || str[1] != 'T' || str[2] != 'O' || str[3] != 'M')
	{
		return PDB_MARK;
	}

	*serialNO = 0;
	char serial[5], resName[4];
	memcpy(serial, str+6, 5);
	memcpy(resName, str+17, 4);
	if(resName[0] != 'T' || resName[1] != 'I' || resName[2] != 'P' || resName[3] != '3')
	{
		return PDB_ATOM;		// not water atom
	}
	*serialNO = atoi(serial);
	return PDB_WATER;
}

// static int *widen_index(int *index, int *index_size, int index_ptr)
// {
// 	int *new_index;
// 	*index_size *= 2;
// 	new_index = (int*)malloc((size_t)index_size * sizeof(*new_index));
// 	if(new_index == NULL)
// 	{
// 		return NULL;
// 	}

// 	memcpy(new_index, index, index_ptr+1);
// 	free(index);
// 	return new_index;
// }