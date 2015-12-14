#include "block.h"

int Block::number=0;

int getLevel(Block* block)
{
    if (block->parent==nullptr) return 0;
    return getLevel(block->parent) + 1;
}

