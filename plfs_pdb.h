#ifndef PLFS_PDB_H
#define PLFS_PDB_H 

#include <stdio.h>

#define PDB_RECORD_LENGTH   80   /* actual record size */
#define PDB_BUFFER_LENGTH   83   /* size need to buffer + CR, LF, and NUL */

#define PDB_MAX_INDEX 		99999

#define PDB_READ	0
#define PDB_WRITE	1

// type of line in .pdb file
#define PDB_MARK 	0		// means this line is a mark
#define PDB_ATOM 	1		// means this line is an atom
#define PDB_WATER 	2		// means this line is a water atom

// Error codes for pdb_errno
#define PDB_SUCCESS			0
#define PDB_BADFORMAT		1
#define PDB_BADPARAMS		2
#define PDB_IOERROR			3
#define PDB_BADMALLOC		4
#define PDB_CANTOPEN		5
#define PDB_CANTCLOSE		6
#define PDB_MODEERROR		7
#define PDB_ERRBUFFULL		8
#define PDB_UNKNOWNERROR	1000

// Error descriptions for pdb_errno
static const char *pdb_errdescs[] = {
	"no error",
	"file does not match format",
	"function called with bad parameters",
	"file i/o error",
	"out of memory",
	"cannot open file",
	"cannot close file",
	"unknown file open mode",
	"error buffer is full",
	NULL
};

// for debug
typedef struct{
	char *buf[20];
	int ptr ;
}pdb_errorbuf;
static pdb_errorbuf pdb_errbuf;

// for debug
static int pdb_seterror(const char *);
int pdb_dispaly_err();


// open and close a .pdb file
static FILE *pdb_open(const char *, const int mode);
static int pdb_close(FILE *);

// get number of all atoms and number of water atomes
int get_pdb_natoms(const char *fn, int *natoms, int *watoms, int **water_index, int *water_ptr);
static int get_atom_type(char *buf, int *serialNO);

// tools
// static int *widen_index(int *index, int *index_size, int index_ptr);

/* ATOM field format according to PDB standard v2.2
  COLUMNS        DATA TYPE       FIELD         DEFINITION
---------------------------------------------------------------------------------
 1 -  6        Record name     "ATOM  "
 7 - 11        Integer         serial        Atom serial number.
13 - 16        Atom            name          Atom name.
17             Character       altLoc        Alternate location indicator.
18 - 20        Residue name    resName       Residue name.
22             Character       chainID       Chain identifier.
23 - 26        Integer         resSeq        Residue sequence number.
27             AChar           iCode         Code for insertion of residues.
31 - 38        Real(8.3)       x             Orthogonal coordinates for X in Angstroms.
39 - 46        Real(8.3)       y             Orthogonal coordinates for Y in Angstroms.
47 - 54        Real(8.3)       z             Orthogonal coordinates for Z in Angstroms.
55 - 60        Real(6.2)       occupancy     Occupancy.
61 - 66        Real(6.2)       tempFactor    Temperature factor.
73 - 76        LString(4)      segID         Segment identifier, left-justified.
77 - 78        LString(2)      element       Element symbol, right-justified.
79 - 80        LString(2)      charge        Charge on the atom.
 */

#endif