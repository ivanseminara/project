require_extension('A');
reg_t address = RS1;
tag_t mem_tag = MMU.tag_read(address);
STORE_TAG_CHECK(mem_tag, address);

if (address == p->get_state()->load_reservation)
{
  MMU.store_uint32(address, RS2);
  WRITE_RD(0);
  // store tag
  MMU.tag_write(address & ~7, 0);
}
else
  WRITE_RD(1);
