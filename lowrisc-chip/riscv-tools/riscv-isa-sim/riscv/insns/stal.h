reg_t temp=RS1 + insn.s_imm();
temp=temp>>6<<6;
MMU.store_dummy( temp );
reg_t i;
for(i=0;i<8;i++) MMU.tag_write( temp+(i<<3), RS2);
