require_extension('C');
require(insn.rvc_rd() != 0);
if (insn.rvc_imm() == 0) { // c.jalr
  reg_t tmp = npc;
  set_pc(RVC_RS1 & ~reg_t(1));
  WRITE_REG(X_RA, tmp,0); //also zero out the register tag
} else {
  WRITE_RD(insn.rvc_imm() << 12);
}
