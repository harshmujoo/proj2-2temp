
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"
#include "tables.h"

const int SYMTBL_NON_UNIQUE = 0;
const int SYMTBL_UNIQUE_NAME = 1;

#define INITIAL_SIZE 5
#define SCALING_FACTOR 2

/*******************************
 * Helper Functions
 *******************************/

void allocation_failed() {
    write_to_log("Error: allocation failed\n");
    exit(1);
}

void addr_alignment_incorrect() {
    write_to_log("Error: address is not a multiple of 4.\n");
}

void name_already_exists(const char* name) {
    write_to_log("Error: name '%s' already exists in table.\n", name);
}

void write_symbol(FILE* output, uint32_t addr, const char* name) {
    fprintf(output, "%u\t%s\n", addr, name);
}

/*******************************
 * Symbol Table Functions
 *******************************/

/* Creates a new SymbolTable containg 0 elements and returns a pointer to that
   table. Multiple SymbolTables may exist at the same time. 
   If memory allocation fails, you should call allocation_failed(). 
   Mode will be either SYMTBL_NON_UNIQUE or SYMTBL_UNIQUE_NAME. You will need
   to store this value for use during add_to_table().
 */
SymbolTable* create_table(int mode) {
    // Allocate memory for table
    SymbolTable * table = malloc(sizeof(SymbolTable));
    if (!table) {
       allocation_failed();
    }

    // Initialize table attributes
    table->len = 0; // how many symbols the table contains
    table->cap = INITIAL_SIZE; // max number of symbols that can be stored using the allocated memory
    table->mode = mode;

    // Add symbols to table
    // Allocate memory for symbols
    Symbol * symbols = malloc(sizeof(Symbol) * INITIAL_SIZE);
    if (!symbols) {
       allocation_failed();
    }

    // Set table pointer to symbols
    table->tbl = symbols;

    return table;
}

/* Frees the given SymbolTable and all associated memory. */
void free_table(SymbolTable* table) {
    Symbol * symbols = table->tbl;
    int len = table->len;

    for (int i = 0; i < len; i++) {
      free(symbols->name);
      symbols++;
    }

    free(table->tbl);
    free(table);
}

/* A suggested helper function for copying the contents of a string. */
static char* create_copy_of_str(const char* str) {
    size_t len = strlen(str) + 1;
    char *buf = (char *) malloc(len);
    if (!buf) {
       allocation_failed();
    }
    strncpy(buf, str, len); 
    return buf;
}

/* Adds a new symbol and its address to the SymbolTable pointed to by TABLE. 
   ADDR is given as the byte offset from the first instruction. The SymbolTable
   must be able to resize itself as more elements are added. 

   Note that NAME may point to a temporary array, so it is not safe to simply
   store the NAME pointer. You must store a copy of the given string.

   If ADDR is not word-aligned, you should call addr_alignment_incorrect() and
   return -1. If the table's mode is SYMTBL_UNIQUE_NAME and NAME already exists 
   in the table, you should call name_already_exists() and return -1. If memory
   allocation fails, you should call allocation_failed(). 

   Otherwise, you should store the symbol name and address and return 0.
 */
int add_to_table(SymbolTable* table, const char* name, uint32_t addr) {
    // Check if address is word aligned
    if (addr % 4 != 0)  { 
      addr_alignment_incorrect();
      return -1;
    }

    // Get table attributes
    Symbol * symbols = table->tbl; // pointer to symbol list
    int len = table->len; // get length of list
    int cap = table->cap; // get max capacity of list
    int mode = table->mode; // mode == SYMTBL_UNIQUE_NAME or SYMTBL_NON_UNIQUE

    // Copy name pointer to allocated memory
    char * name_copy = create_copy_of_str(name);

    // Check if table is full
    if (len == cap) {
      //printf("%s", "Resizing");
      // Allocate more memory
      table->tbl = (Symbol *) realloc(table->tbl, (sizeof(Symbol) * (cap * SCALING_FACTOR)));
      if (!table->tbl) {
        allocation_failed();
      }
      table->cap = cap * SCALING_FACTOR;
      symbols = table->tbl;
    }

    // Iterate over symbols array 
    for (int i = 0; i < len; i++) {
        char * current_name = symbols->name;
        // check if entry with same name already exists
        if (mode == SYMTBL_UNIQUE_NAME && strcmp(current_name, name) == 0)  { 
          name_already_exists(current_name);
          return -1;
        }
        symbols++; // point to next symbol 
    }

    Symbol newSymbol;
    newSymbol.name = name_copy;
    newSymbol.addr = addr;

    *symbols = newSymbol;
    table->len += 1;

    return 0;
}

/* Returns the address (byte offset) of the given symbol. If a symbol with name
   NAME is not present in TABLE, return -1.
 */
int64_t get_addr_for_symbol(SymbolTable* table, const char* name) {
    Symbol * symbols = table->tbl; // pointer to symbol list
    int len = table->len; // get length of list

    for (int i = 0; i < len; i++) {
      char * current = symbols->name;  
      if (strcmp(current, name) == 0)  { // compare each entry to name
        return symbols->addr;
      }
      symbols++;  // point to the next element in the list
    }

    return -1;   
}

/* Writes the SymbolTable TABLE to OUTPUT. You should use write_symbol() to
   perform the write. Do not print any additional whitespace or characters.
 */
void write_table(SymbolTable* table, FILE* output) {
    int len = table->len;
    Symbol * symbols = table->tbl;

    for (int i = 0; i < len; i++) {
      //printf("%s\n", "made it this far");
      write_symbol(output, table->tbl[i].addr, table->tbl[i].name);
      //symbols++;
    }
}
