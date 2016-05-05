require_extension('A');
require_rv64;
reg_t address = RS1;
tag_t mem_tag = MMU.tag_read(address);
LOAD_STORE_TAG_CHECK(mem_tag, address);
sreg_t v = MMU.load_int64(address);
MMU.store_uint64(address, std::min(sreg_t(RS2),v));
// store tag
MMU.tag_write(address, 0);
WRITE_RD(v);
