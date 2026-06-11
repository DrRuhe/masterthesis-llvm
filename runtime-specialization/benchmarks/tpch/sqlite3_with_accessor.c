/*
 * sqlite3 amalgamation wrapper that exposes internal Vdbe fields needed by the
 * input-size-limits benchmark (spec 018). Compiled in the same TU as sqlite3.c
 * so the Vdbe and VdbeOp struct definitions are accessible.
 */
#include "sqlite3.c"

/* Returns the number of bytecode ops in a prepared statement. */
int crs_vdbe_nop(sqlite3_stmt *stmt) {
    Vdbe *v = (Vdbe *)stmt;
    return v->nOp;
}

/* Returns sizeof(VdbeOp) for computing total bytecode size in bytes. */
int crs_vdbe_op_size(void) {
    return (int)sizeof(VdbeOp);
}
