#include "plfs_pdb.h"
#include <string.h>



const char *fileName = "../test_data/test.pdb";

int main(int argc, char const *argv[])
{
	int natoms, watoms, water_ptr;
	int *water_index;
	if(get_pdb_natoms(fileName, &natoms, &watoms, &water_index, &water_ptr))
	{
		pdb_dispaly_err();
		return 0;
	}

	pdb_dispaly_err();

	printf("*** %d    %d\n", natoms, watoms);
	if(water_index && water_ptr)
	{
		printf("%d\n", water_ptr);
		for(int i = 0; i < water_ptr; ++i)
			printf("%d  ", water_index[i]);
		printf("\n");
	}
	printf("*** %d    %d\n", natoms, watoms);

	return 0;
}