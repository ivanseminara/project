//MMU.load_dummy( RS1 + insn.i_imm() );
WRITE_RD( MMU.tag_read(RS1 + insn.i_imm()) );
