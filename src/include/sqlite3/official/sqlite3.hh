#ifndef __OFFICIAL_SQLITE3_HH__
#define __OFFICIAL_SQLITE3_HH__

#include "base.hh"

/*
** The "context" argument for an installable function.  A pointer to an
** instance of this structure is the first argument to the routines used
** implement the SQL functions.
**
** There is a typedef for this structure in sqlite.h.  So all routines,
** even the public interface to SQLite, can use a pointer to this structure.
** But this file is the only place where the internal details of this
** structure are known.
**
** This structure is defined inside of vdbeInt.h because it uses substructures
** (Mem) which are only defined there.
*/
struct sqlite3_context {
  Mem *pOut;              /* The return value is stored here */
  FuncDef *pFunc;         /* Pointer to function information */
  Mem *pMem;              /* Memory cell used to store aggregate context */
  Vdbe *pVdbe;            /* The VM that owns this context */
  int iOp;                /* Instruction number of OP_Function */
  int isError;            /* Error code returned by the function. */
  u8 enc;                 /* Encoding to use for results */
  u8 skipFlag;            /* Skip accumulator loading if true */
  u8 argc;                /* Number of arguments */
  sqlite3_value *argv[1]; /* Argument set */
};

/*
** Internally, the vdbe manipulates nearly all SQL values as Mem
** structures. Each Mem struct may cache multiple representations (string,
** integer etc.) of the same value.
*/
struct sqlite3_value {
  union MemValue {
    double r;           /* Real value used when MEM_Real is set in flags */
    i64 i;              /* Integer value used when MEM_Int is set in flags */
    int nZero;          /* Extra zero bytes when MEM_Zero and MEM_Blob set */
    const char *zPType; /* Pointer type when MEM_Term|MEM_Subtype|MEM_Null */
    FuncDef *pDef;      /* Used only when flags==MEM_Agg */
  } u;
  char *z;            /* String or BLOB value */
  int n;              /* Number of characters in string value, excluding '\0' */
  u16 flags;          /* Some combination of MEM_Null, MEM_Str, MEM_Dyn, etc. */
  u8  enc;            /* SQLITE_UTF8, SQLITE_UTF16BE, SQLITE_UTF16LE */
  u8  eSubtype;       /* Subtype for this value */
  /* ShallowCopy only needs to copy the information above */
  void /*sqlite3*/ *db;        /* The associated database connection */
  int szMalloc;       /* Size of the zMalloc allocation */
  u32 uTemp;          /* Transient storage for serial_type in OP_MakeRecord */
  char *zMalloc;      /* Space to hold MEM_Str or MEM_Blob if szMalloc>0 */
  void (*xDel)(void*);/* Destructor for Mem.z - only valid if MEM_Dyn */
 #ifdef _WIN32// && SQLITE_DEBUG
    Mem *pScopyFrom;    /* This Mem is a shallow copy of pScopyFrom */
    u16 mScopyFlags;    /* flags value immediately after the shallow copy */
 #endif
};

#endif