require_extension('C');
if (insn.rvc_rd() == 0) { // c.addi16sp
  WRITE_REG(X_SP, sext_xlen(RVC_SP + insn.rvc_addi16sp_imm()), 0); // also zero out the register tag
} else {
  require(insn.rvc_imm() != 0);
  WRITE_RD(sext_xlen(RVC_RS1 + insn.rvc_imm()));
}
