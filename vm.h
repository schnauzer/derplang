/*
 * Implements the Cream virtual machine
 * This includes the mark phase of garbage collection
*/

#ifndef CREAM_VM_H
#define CREAM_VM_H

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "list.h"
#include "map.h"
#include "utils.h"
#include "debug.h"
#include "object.h"
#include "bytecodes.h"

#define MAX_LINE_LEN 64
#define MAX_IDENT_LEN 16

typedef enum {
  OP_ADD,
  OP_SUB,
  OP_MUL,
  OP_DIV,
  OP_CMP_EQ,
  OP_CMP_NEQ,
  OP_CMP_LT,
  OP_CMP_LT_EQ,
  OP_CMP_GT,
  OP_CMP_GT_EQ,
} Vm_arithmetic_optype;

typedef struct {
  int return_addr;
  Map* symbol_table;
} Stack_frame;

typedef struct {
  List* stack;
  List* call_stack; // list of Stack_frames
  Map* func_map;

  bool err;

  int pointer;
  instr* bytecode;
  size_t num_bytecodes;

  List* std_lib;
} Cream_vm;

typedef void (*Cream_native_fn)(Cream_vm* vm, int argc);

typedef struct {
  char* name;
  Cream_native_fn fn;
} Cream_native;

#include "stdlib.h"

/* allocates and initializes a vm, then returns a pointer to it */
Cream_vm* cream_vm_create();

/* deallocates a vm and all its associated data */
void cream_vm_destroy(Cream_vm *obj);

/* add a function to the cream standard library */
void cream_add_native(Cream_vm* vm, char* name, Cream_native_fn fn);

/* call the standard libary function with the given name, with arguments
 * coming from the vm's stack */
bool cream_run_native(Cream_vm* vm, char* name, int argc);

/* stop the world, then mark all objects reachable from the current scope */
void vm_gc_mark(Cream_vm *vm);

/* run the vm */
void cream_vm_run(Cream_vm *vm);

#endif
