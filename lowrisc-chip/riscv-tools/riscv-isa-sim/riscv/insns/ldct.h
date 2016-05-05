require_rv64;

reg_t address = RS1 + insn.i_imm();
tag_t mem_tag = MMU.tag_read(address);
LOAD_TAG_CHECK(mem_tag, address);

WRITE_REG(insn.rd(), MMU.load_int64(address), mem_tag);
