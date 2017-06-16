#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <string>
#include <iostream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>


#define pdbName "H:/Data-for-VMD/water-channel/apo63run3-ex3-2769000.pdb"

using namespace std;

int ana_buf(char *buf, int buf_size, int *natoms, vector<int> *pp);

int ana_buf(char *buf, int buf_size, int *natoms, vector<int> *pp)
{
	char ser[6];
	int wno;
	for(int i = 0; i < buf_size; ++i)
	{
		if(buf[i] == 'A' && buf[i+1] == 'T' && buf[i+2] == 'O' && buf[i+3] == 'M')
			++(*natoms);
		if(buf[i] == 'T' && buf[i+1] == 'I' && buf[i+2] == 'P' && buf[i+3] == '3')
		{
			memcpy(ser, buf+i-11, 5);
			wno = atoi(ser);
			if(pp->size() == 0){
				pp->push_back(wno);
			}else if(pp->size() % 2){
				if(pp->back() == (wno - 1)){
					pp->push_back(wno);
				}else{
					pp->push_back(pp->back());
					pp->push_back(wno);
				}
			}else{
				if(pp->back() == (wno - 1)){
					// (*pp)[pp->size() - 1] = wno;
					pp->pop_back();
					pp->push_back(wno);
				}else{
					pp->push_back(wno);
				}
			}
		}
	}

	if(pp->size() % 2){
		pp->push_back(pp->back());
	}
	return 0;
}

int main()
{
	int fd, ret;
	struct stat fst;
	char *buf;

	fd = open(pdbName, O_RDONLY);
	fstat(fd, &fst);

	cout << "pdb file's length: " << fst.st_size << endl;

	buf = (char*)malloc(fst.st_size + 1);
	if(buf == NULL){
		cout << "buf == NULL" << endl;
		close(fd);
		return 0;
	}
	ret = pread(fd, buf, fst.st_size, 0);
	if(ret != fst.st_size){
		cout << "ret != fst.st_size" << endl;
		close(fd);
		return 0;
	}

	// int natoms, wn;
	// char ser[6];
	// for(int i = 0; i < fst.st_size - 5; ++ i){
	// 	if(buf[i] == 'A' && buf[i+1] == 'T' && buf[i+2] == 'O' && buf[i+3] == 'M')
	// 		++natoms;
	// 	if(buf[i] == 'T' && buf[i+1] == 'I' && buf[i+2] == 'P' && buf[i+3] == '3')
	// 	{
	// 		++wn;
	// 		memcpy(ser, buf+i-11, 6);
	// 		cout << ser << endl;
	// 		int zz = atoi(ser);
	// 		cout << zz << endl;
	// 		break;
	// 	}
	// }
	// cout << "natoms: " << natoms << endl;
	// cout << "wn: " << wn << endl;

	close(fd);

	vector<int> pp;
	int natoms;
	ana_buf(buf, fst.st_size, &natoms, &pp);

	delete buf;

	cout << natoms << endl;
	for(int i = 0; i < pp.size(); ++i){
		cout << pp[i] << endl;
	}


	return 0;
}

