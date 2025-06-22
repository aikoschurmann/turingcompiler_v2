#include "cfg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tac_print.h"

void add_successor(CFGBlock *from, CFGBlock *to) {
    if (from == NULL || to == NULL) {
        printf("Error: Attempted to add a successor or predecessor to a NULL block.\n");
        exit(EXIT_FAILURE);
    }
    push_block(&from->successors, to);
}

void add_predecessor(CFGBlock *to, CFGBlock *from) {
    if (to == NULL || from == NULL) {
        printf("Error: Attempted to add a successor or predecessor to a NULL block.\n");
        exit(EXIT_FAILURE);
    }
    push_block(&to->predecessors, from);
}


void push_block(CFGBlockList *list, CFGBlock *block) {
    if (list->count >= list->capacity) {
        size_t new_capacity = list->capacity ? list->capacity * 2 : 4;
        CFGBlock **new_items = realloc(list->items, new_capacity * sizeof(CFGBlock *));
        if (!new_items) {
            printf("Memory allocation failed while resizing CFGBlockList.\n");
            exit(EXIT_FAILURE);
        }
        list->items = new_items;
        list->capacity = new_capacity;
    }
    list->items[list->count++] = block;
}

CFGBlock *create_block(int id, int is_entry, int is_exit) {
    CFGBlock *block = calloc(1, sizeof(CFGBlock));
    if (block == NULL) {
        printf("Memory allocation failed for CFGBlock.\n");
        exit(EXIT_FAILURE);
    }
    block->id = id;
    return block;
}

void push_block_array(CFGBlockArray *array,  CFGBlock *block) {
    if (array->count >= array->capacity) {
        size_t new_capacity = array->capacity ? array->capacity * 2 : 4;
        CFGBlock **new_items = realloc(array->items, new_capacity * sizeof(CFGBlock *));
        if (!new_items) {
            printf("Memory allocation failed while resizing CFGBlockArray.\n");
            exit(EXIT_FAILURE);
        }
        array->items = new_items;
        array->capacity = new_capacity;
    }
    array->items[array->count++] = block;
}

void init_cfg(CFG *cfg) {
    cfg->blocks.items = NULL;
    cfg->blocks.count = 0;
    cfg->blocks.capacity = 0;
}

void free_cfg(CFG *cfg) {
    if (cfg == NULL) {
        return; 
    }
    free_cfg_block_array(&cfg->blocks);
}

void free_cfg_block(CFGBlock *block) {
    if (block == NULL) {
        return; 
    }
    free(block->instructions);
    free_cfg_block_list(&block->successors);
    free_cfg_block_list(&block->predecessors);
    free(block);
}

void free_cfg_block_list(CFGBlockList *list) {
    if (list == NULL) {
        return; 
    }
    for (size_t i = 0; i < list->count; i++) {
        free_cfg_block(list->items[i]);
    }
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

void free_cfg_block_array(CFGBlockArray *array) {
    if (array == NULL) {
        return; 
    }
    for (size_t i = 0; i < array->count; i++) {
        free_cfg_block(array->items[i]);
    }
    free(array->items);
    array->items = NULL;
    array->count = 0;
    array->capacity = 0;
}

CFG *create_cfg(void) {
    CFG *cfg = malloc(sizeof(CFG));
    if (cfg == NULL) {
        printf("Memory allocation failed for CFG.\n");
        exit(EXIT_FAILURE);
    }
    init_cfg(cfg);
    return cfg;
}

void print_cfg(CFG *cfg) {
    if (cfg == NULL) {
        printf("CFG is NULL.\n");
        return;
    }
    printf("CFG with %zu blocks:\n", cfg->blocks.count);
    for (size_t i = 0; i < cfg->blocks.count; i++) {
        CFGBlock *block = cfg->blocks.items[i];
        printf("Block ID: %d, Entry: %d, Exit: %d\n", block->id, block->is_entry, block->is_exit);
        tac_print_list(block->instructions);
        printf("\n");
    }
}