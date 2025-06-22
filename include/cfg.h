#pragma once

#include <stddef.h>
#include "tac.h"


typedef struct CFGBlock CFGBblock;

typedef struct CFGBlockList {
    struct CFGBlock **items;
    size_t count;
    size_t capacity;
} CFGBlockList;

typedef struct CFGBlock {
    int id;
    int is_entry;
    int is_exit;
    TACInstr *instructions;

    CFGBlockList successors;
    CFGBlockList predecessors;
} CFGBlock;

typedef struct CFGBlockArray {
    CFGBlock **items;
    size_t count;
    size_t capacity;
} CFGBlockArray;

typedef struct CFG {
    CFGBlockArray blocks;
} CFG;


void add_successor(CFGBlock *from, CFGBlock *to);
void add_predecessor(CFGBlock *to, CFGBlock *from);
void push_block(CFGBlockList *list, CFGBlock *block);

CFGBlock *create_block(int id, int is_entry, int is_exit);

void push_block_array(CFGBlockArray *array, CFGBlock *block);
void init_cfg(CFG *cfg);
void free_cfg(CFG *cfg);
void free_cfg_block(CFGBlock *block);
void free_cfg_block_list(CFGBlockList *list);
void free_cfg_block_array(CFGBlockArray *array);

CFG *create_cfg(void);
void print_cfg(CFG *cfg);