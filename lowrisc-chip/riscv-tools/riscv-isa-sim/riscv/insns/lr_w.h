require_extension('A');
reg_t address = RS1;
tag_t mem_tag = MMU.tag_read(address);
LOAD_TAG_CHECK(mem_tag, address);
p->get_state()->load_reservation = address;
WRITE_RD(MMU.load_int32(address));