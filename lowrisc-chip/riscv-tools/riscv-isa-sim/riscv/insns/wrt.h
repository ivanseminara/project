reg_t value = RS2;
tag_t tag_value = value & ((1<<TAG_SIZE)-1);
WRITE_REG(insn.rd(), RS1, tag_value);
