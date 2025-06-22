#include "cfg_builder.h"
#include <stdlib.h>
#include <stdio.h>

// Helper: Determine if a TAC instruction ends a basic block
int is_block_terminator(TACInstr *instr) {
    return  instr->kind==TAC_GOTO 
                  || instr->kind==TAC_IFZ
                  || instr->kind==TAC_RETURN
                  || instr->kind==TAC_END_FUNCTION;
}


CFGBlock *create_block_from_range(CFG *cfg, TACInstr *start, TACInstr *end, int id) {
    CFGBlock *block = create_block(id, 0, 0); 
    if (!block) return NULL;

    block->instructions = start;

    // Null-terminate the block's instruction list
    if (end && end->next) {
        end->next = NULL;
    }

    // Add the block to the CFG
    push_block_array(&cfg->blocks, block);
    return block;
}

// Main function
CFG *build_from_tac(TACInstr *tac) {
    CFG *cfg = create_cfg();  // Allocates and initializes CFG
    if (!cfg) return NULL;

    int block_id = 0;
    TACInstr *cursor = tac;
    TACInstr *block_start = tac;

    while (cursor) {
        TACInstr *next = cursor->next;

        // If the next instruction is a label, we must start a new block
        if (next && next->kind == TAC_LABEL) {
            CFGBlock *block = create_block_from_range(cfg, block_start, cursor, block_id++);
            if (!block) {
                free_cfg(cfg);
                return NULL;
            }
            block->is_exit = (cursor->kind == TAC_RETURN || cursor->kind == TAC_END_FUNCTION);
            block_start = next;
        }

        // If current instruction is a block terminator
        else if  (is_block_terminator(cursor)) {
            CFGBlock *block = create_block_from_range(cfg, block_start, cursor, block_id++);
            if (!block) {
                free_cfg(cfg);
                return NULL;
            }
            block->is_exit = (cursor->kind == TAC_RETURN || cursor->kind == TAC_END_FUNCTION);
            block_start = next;
        }

        cursor = next;
    }

    // Catch the last block if it wasn’t terminated (e.g., a fall-through)
    //if (block_start) {
    //    CFGBlock *block = create_block_from_range(cfg, block_start, NULL, block_id++);
    //    if (!block) {
    //        free_cfg(cfg);
    //        return NULL;
    //    }
    //}

    return cfg;
}

