#include "plfs_fuse_xtc.h"
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

#define xtcName "H:/Data-for-VMD/water-channel/step7_extend-400ns-fit-390ns-allwater.xtc"
#define pdbName "H:/Data-for-VMD/water-channel/apo63run3-ex3-2769000.pdb"
#define writeName "H:/Data-for-VMD/water-channel/git-del-write-test-01.xtc"
#define writeXTC "H:/Data-for-VMD/water-channel/git-del-write-test-xtc.xtc"

char xtc_zero[4] = {0, 0, 0, 0};

#define RSIZE	(256 * 512)

Pxtc *Pxtc::self;


int cp_file();
int read_file();
int write_xtc(const char *buf, size_t size);


int main()
{
	// cp_file();
	int fd = open(writeXTC, O_CREAT, S_IRWXG | S_IRWXO | S_IRWXU);
	close(fd);
	read_file();

	return 0;
}

int read_file()
{
	Pxtc px;
	px.init_self(&px);

	char *buf;
	int magic, natoms, bytecnt, ret;

	int count = 0;
	int offset = 0, size;

	while(1)
	{
		++count;
		if(count > 20)
			break;

		// read header;
		buf = (char*)malloc(92);
		ret = px.read_file(writeName, buf, 92, offset);
		if(ret != 0)
		{
			return 0;
		}
		offset += 92;
		px.get_int(buf, &magic);
		px.get_int(buf + 4, &natoms);
		px.get_int(buf + 88, &bytecnt);
		cout << "#### magic: " << magic
			 << "  natoms: " << natoms
			 << "  bytecnt: " << bytecnt
			 << endl;
		write_xtc(buf, 92);
		free(buf);

		// read coordnate;
		int bsize = bytecnt;
		buf = (char*)malloc(bsize);
		ret = px.read_file(writeName, buf, bsize, offset);
		if(ret != 0)
		{
			return 0;
		}
		offset += (bsize % 4) ? (bsize + (4 - bsize % 4)) : (bsize);
		write_xtc(buf, bytecnt);
		free(buf);
		if(bytecnt % 4){
			write_xtc(xtc_zero, (4 - bytecnt % 4));
		}
		cout << "\n\n\n" << endl;
		// break;
	}

	return 0;
}

int write_xtc(const char *buf, size_t size)
{
	int fd;
	fd = open(writeXTC, O_RDWR);
	int ret = 0;
	lseek(fd, 0, SEEK_END);
	ret = write(fd, buf, size);
	if(ret != size){
		cout << "write error!" << endl;
		exit(1);
	}
	close(fd);
	return ret;
}

int cp_file()
{
	FILE *fp;
	fp = fopen(xtcName, "rb");

	size_t f_length;
	fseek(fp, 0, SEEK_END);
	f_length = ftell(fp);
	rewind(fp);

	size_t cur_size = 0, read_size;
	off_t cur_off = 0;
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

		// cout << "MMMM " << read_size << endl;
		buf = (char*)malloc(read_size * sizeof(char));

		if(fread(buf, read_size, 1, fp) == 1)
		{
			ret = px.write_file(writeName, buf, read_size, cur_off);
			if(ret == 0){
				cout << " 0: hold" << endl;;
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
			}else if(ret < -90){
				cout << "op error: " << ret - PLFS_TBD << endl;
				break;
			}else{
				cout << "WTF!!" << endl;
				break;
			}
			cur_size += read_size;
			cur_off += read_size;
			free(buf);
		}else{
			cout << "fread error!" << endl;
			free(buf);
			break;
		}

	}

	fclose(fp);
}

