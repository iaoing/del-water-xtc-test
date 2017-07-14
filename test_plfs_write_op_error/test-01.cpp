#include "plfs_fuse_xtc.h"
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>

using namespace std;

#define xtcName "H:/Data-for-VMD/water-channel/step7_extend-400ns-fit-390ns-allwater.xtc"
#define pdbName "H:/Data-for-VMD/water-channel/apo63run3-ex3-2769000.pdb"
#define writeName "H:/Data-for-VMD/water-channel/git-del-write-test-01.xtc"

#define RSIZE	(256 * 512)

Pxtc *Pxtc::self;

int main()
{
	FILE *fp;
	fp = fopen(xtcName, "rb");

	size_t f_length;
	fseek(fp, 0, SEEK_END);
	f_length = ftell(fp);
	rewind(fp);

	size_t cur_size = 0, read_size;
	off_t cur_off;
	char *buf;

	int ret;

	Pxtc px;
	px.init_self(&px);

	while(cur_size < f_length)
	{
		if(cur_size + RSIZE <= f_length)
			read_size = RSIZE;
		else
			read_size = f_length - cur_size;

		buf = (char*)malloc(read_size * sizeof(char));

		if(fread(buf, read_size, 1, fp) == 1)
		{
			ret = px.write_file(writeName, buf, read_size, cur_off);
			if(ret == 0){
				cout << "0: hold" << endl;;
			}else if(ret == -3){
				cout << "-3: fluse buffer, then f_write" << endl;
			}else if(ret == -2){
				cout << "-2: flush buffer, return ditectly" << endl;
			}else if(ret == -1){
				cout << "-1: goto f_write" << endl;
			}else if(ret == 1){
				cout << " 1: flush frame" << endl;
			}else if(ret == 2){
				cout << " 2: flush trunc frame" << endl;
			}else{
				cout << "WTF!!" << endl;
				break;
			}
			cur_size += read_size;
			cur_off += read_size;
			free(buf);
		}else{
			cout << "fread error!" << endl;
			break;
		}

	}

	fclose(fp);
	free(buf);

	return 0;
}


