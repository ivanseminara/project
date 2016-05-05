require_rv64;
reg_t address = RS1 + insn.s_imm();
tag_t mem_tag = MMU.tag_read(address);
STORE_TAG_CHECK(mem_tag, address);
// store data
MMU.store_uint64(address, RS2);
// store tag
MMU.tag_write(address, 0);
