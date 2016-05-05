require_extension('A');
reg_t address = RS1;
tag_t mem_tag = MMU.tag_read(address);
LOAD_STORE_TAG_CHECK(mem_tag, address);
reg_t v = MMU.load_int32(address);
MMU.store_uint32(address, RS2 | v);
// store tag
MMU.tag_write(address & ~7, 0);
WRITE_RD(v);
