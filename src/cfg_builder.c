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

    return cfg;
}

CFG *extract_functions(TACInstr *tac) {
    CFG *cfg = create_cfg();
    if (!cfg) {
        fprintf(stderr, "Failed to create CFG.\n");
        return NULL;
    }

    int id = 0;
    TACInstr *cursor = tac;
    TACInstr *seg_start = tac;
    TACInstr *prev = NULL;
    int depth = 0;


    while (cursor) {
        switch (cursor->kind) {
            case TAC_FUNCTION:
                // Close previous segment (global code or enclosing function body)
                if (seg_start && seg_start != cursor && depth == 0) {
                    if (!create_block_from_range(cfg, seg_start, prev, id++)) {
                        free_cfg(cfg);
                        return NULL;
                    }
                }
                depth++;
                // Start new segment
                seg_start = cursor;
                break;

            case TAC_END_FUNCTION:
                depth--;
                if (depth < 0) {
                    fprintf(stderr, "END_FUNCTION without matching FUNCTION.\n");
                    free_cfg(cfg);
                    return NULL;
                }
                // Close the function or nested function body
                if (seg_start && seg_start != cursor && depth == 0) {
                    if (!create_block_from_range(cfg, seg_start, cursor, id++)) {
                        free_cfg(cfg);
                        return NULL;
                    }
                    // Next segment starts after
                    seg_start = cursor->next;
                }
   
                break;

            default:
                // Regular instruction, continue
                break;
        }
        prev = cursor;
        cursor = cursor->next;
    }


    return cfg;
}
