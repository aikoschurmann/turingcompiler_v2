#pragma once
#include "cfg.h"

CFG *build_from_tac(TACInstr *tac);

void free_cfg_builder(CFG *cfg);
CFG *extract_functions(TACInstr *tac);