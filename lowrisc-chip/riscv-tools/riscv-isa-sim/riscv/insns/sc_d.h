require_extension('A');
require_rv64;
reg_t address = RS1;
tag_t mem_tag = MMU.tag_read(address);
STORE_TAG_CHECK(mem_tag, address);
if (address == p->get_state()->load_reservation)
{
  // store data
  MMU.store_uint64(address, RS2);
  // store tag
  MMU.tag_write(address, 0);
  WRITE_RD(0);
}
else
  WRITE_RD(1);
