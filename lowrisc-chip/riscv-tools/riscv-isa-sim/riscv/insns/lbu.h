reg_t address = RS1 + insn.i_imm();
tag_t mem_tag = MMU.tag_read(address);
LOAD_TAG_CHECK(mem_tag, address);
WRITE_RD(MMU.load_uint8(address));
