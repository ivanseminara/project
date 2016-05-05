MMU.load_dummy( (RS1 + insn.s_imm())>>3<<3 );
MMU.tag_write(RS1 + insn.s_imm(), RS2);
