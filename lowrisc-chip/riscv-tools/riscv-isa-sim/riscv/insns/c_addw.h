require_extension('C');
require_rv64;
require(insn.rvc_rd() != 0 && insn.rvc_rs2() != 0);
WRITE_RD(sext32(RVC_RS1 + RVC_RS2));
