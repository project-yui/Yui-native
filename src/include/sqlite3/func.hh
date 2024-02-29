#ifndef __FUNC_HH__
#define __FUNC_HH__

#include "base.hh"

/*
** Each SQL function is defined by an instance of the following
** structure.  For global built-in functions (ex: substr(), max(), count())
** a pointer to this structure is held in the sqlite3BuiltinFunctions object.
** For per-connection application-defined functions, a pointer to this
** structure is held in the db->aHash hash table.
**
** The u.pHash field is used by the global built-ins.  The u.pDestructor
** field is used by per-connection app-def functions.
*/
struct FuncDef {
  i8 nArg;             /* Number of arguments.  -1 means unlimited */
  u32 funcFlags;       /* Some combination of SQLITE_FUNC_* */
  void *pUserData;     /* User data parameter */
  FuncDef *pNext;      /* Next function with same name */
  void (*xSFunc)(sqlite3_context*,int,sqlite3_value**); /* func or agg-step */
  void (*xFinalize)(sqlite3_context*);                  /* Agg finalizer */
  void (*xValue)(sqlite3_context*);                     /* Current agg value */
  void (*xInverse)(sqlite3_context*,int,sqlite3_value**); /* inverse agg-step */
  const char *zName;   /* SQL name of the function. */
  union {
    FuncDef *pHash;      /* Next with a different name but the same hash */
    FuncDestructor *pDestructor;   /* Reference counted destructor function */
  } u; /* pHash if SQLITE_FUNC_BUILTIN, pDestructor otherwise */
};

#endif