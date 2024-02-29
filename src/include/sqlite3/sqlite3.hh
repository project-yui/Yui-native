#ifndef __SQLITE3_HH__
#define __SQLITE3_HH__

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
struct nt_sqlite3_context {
  NTMem *pOut;              /* The return value is stored here */
  FuncDef *pFunc;         /* Pointer to function information */
  NTMem *pMem;              /* Memory cell used to store aggregate context */
  NTVdbe *pVdbe;            /* The VM that owns this context */
  int iOp;                /* Instruction number of OP_Function */
  int isError;            /* Error code returned by the function. */
  u8 enc;                 /* Encoding to use for results */
  u8 skipFlag;            /* Skip accumulator loading if true */
  u8 argc;                /* Number of arguments */
  nt_sqlite3_value *argv[1]; /* Argument set */
};

/**
 * @brief 
 * 
 * nt 内存中大小为56字节
 */
struct nt_sqlite3_value {
  union MemValue {
    double r;           /* Real value used when MEM_Real is set in flags */
    i64 i;              /* Integer value used when MEM_Int is set in flags */
    int nZero;          /* Extra zero bytes when MEM_Zero and MEM_Blob set */
    const char *zPType; /* Pointer type when MEM_Term|MEM_Subtype|MEM_Null */
    FuncDef *pDef;      /* Used only when flags==MEM_Agg */
  } u;
  u16 flags;          /* Some combination of MEM_Null, MEM_Str, MEM_Dyn, etc. */
  /**
   * @brief 长度 nt内存偏移12
   * 
   */
  int n;              /* Number of characters in string value, excluding '\0' */
  /**
   * @brief 字符串 nt内存偏移16
   * 
   */
  char *z;            /* String or BLOB value */
  char *zMalloc;      /* Space to hold MEM_Str or MEM_Blob if szMalloc>0 */
  u8  enc;            /* SQLITE_UTF8, SQLITE_UTF16BE, SQLITE_UTF16LE */
  u8  eSubtype;       /* Subtype for this value */
  /* ShallowCopy only needs to copy the information above */
  /**
   * @brief nt 内存偏移40
   * 
   */
  void /*sqlite3*/ *db;        /* The associated database connection */
  int szMalloc;       /* Size of the zMalloc allocation */
  u32 uTemp;          /* Transient storage for serial_type in OP_MakeRecord */
  // void (*xDel)(void*);/* Destructor for Mem.z - only valid if MEM_Dyn */
#ifdef SQLITE_DEBUG
  // void /*Mem*/ *pScopyFrom;    /* This Mem is a shallow copy of pScopyFrom */
  // u16 mScopyFlags;    /* flags value immediately after the shallow copy */
#endif
};

#endif