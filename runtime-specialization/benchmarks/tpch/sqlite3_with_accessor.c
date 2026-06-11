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

/*
 * Set Vdbe::pc so that a direct sqlite3VdbeExec(vdbe) call starts from the
 * correct instruction. sqlite3_reset() leaves pc=-1; sqlite3Step() normally
 * resets it to 0 before calling sqlite3VdbeExec. Call this with pc=0 after
 * sqlite3_reset when invoking sqlite3VdbeExec directly (e.g. from a
 * specialised wrapper that bypasses sqlite3Step).
 */
void crs_vdbe_set_pc(sqlite3_stmt *stmt, int pc) {
    Vdbe *v = (Vdbe *)stmt;
    v->pc = pc;
}
