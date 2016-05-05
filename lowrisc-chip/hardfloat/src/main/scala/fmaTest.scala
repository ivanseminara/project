// See LICENSE for license details.

package hardfloat

import Chisel._
import Node._
import util.Random
import scala.sys.process._

class FMA(val sigWidth: Int, val expWidth: Int)  extends Module {
  val io = new Bundle {
    val a = Bits(INPUT, sigWidth + expWidth)
    val b = Bits(INPUT, sigWidth + expWidth)
    val c = Bits(INPUT, sigWidth + expWidth)
    val out = Bits(OUTPUT, sigWidth + expWidth)
  }

  val fma = Module(new mulAddSubRecodedFloatN(sigWidth, expWidth))
  fma.io.op := UInt(0)
  fma.io.a := floatNToRecodedFloatN(io.a, sigWidth, expWidth)
  fma.io.b := floatNToRecodedFloatN(io.b, sigWidth, expWidth)
  fma.io.c := floatNToRecodedFloatN(io.c, sigWidth, expWidth)
  fma.io.roundingMode := UInt(0)
  io.out := recodedFloatNToFloatN(fma.io.out, sigWidth, expWidth)
}

class FMAPipeline(sigWidth: Int, expWidth: Int) extends Module {
  val io = new Bundle {
    val req = Bool(INPUT)
    val resp = Bool(OUTPUT)
    val a = Bits(INPUT, sigWidth + expWidth + 1)
    val b = Bits(INPUT, sigWidth + expWidth + 1)
    val c = Bits(INPUT, sigWidth + expWidth + 1)
    val out = Bits(OUTPUT, sigWidth + expWidth + 1 + 5)
  }
  val fma = Module(new mulAddSubRecodedFloatN(sigWidth, expWidth))
  fma.io.a := io.a
  fma.io.b := io.b
  fma.io.c := io.c
  fma.io.op := Bits(0)
  fma.io.roundingMode := io.a
  io.out := RegEnable(Cat(fma.io.exceptionFlags, fma.io.out), io.req)
  io.resp := Reg(next=io.req)
}

class FMARecoded(val sigWidth: Int, val expWidth: Int)  extends Module {
  val io = new Bundle {
    val a = Bits(INPUT, sigWidth + expWidth + 1)
    val b = Bits(INPUT, sigWidth + expWidth + 1)
    val c = Bits(INPUT, sigWidth + expWidth + 1)
    val out = Bits(OUTPUT, sigWidth + expWidth + 1 + 5)
  }

  val fma = Module(new FMAPipeline(sigWidth, expWidth))
  val en = io.a.orR
  fma.io.req := Reg(next=en, init=Bool(false))
  fma.io.a := RegEnable(io.a, en)
  fma.io.b := RegEnable(io.b, en)
  fma.io.c := RegEnable(io.c, en)
  io.out := RegEnable(fma.io.out, fma.io.resp)
}

class SFMARecoded extends FMARecoded(23, 9)
class DFMARecoded extends FMARecoded(52, 12)

class RAM(val w: Int, val d: Int, val nr: Int) extends Module {
  val io = new Bundle {
    val wa = Bits(INPUT, log2Up(d))
    val we = Bool(INPUT)
    val wd = Bits(INPUT, w)

    val ra = Vec.fill(nr)(Bits(INPUT, log2Up(d)))
    val re = Vec.fill(nr)(Bool(INPUT))
    val rd = Vec.fill(nr)(Bits(OUTPUT, w))
  }

  val ram = Mem(Bits(width = w), d)
  when (io.we) { ram(io.wa) := io.wd }

  for (i <- 0 until nr) {
    val ra = RegEnable(io.ra(i), io.re(i))
    val re = Reg(next=io.re(i), init=Bool(false))
    io.rd(i) := RegEnable(ram(ra), re)
  }
}

class FMATests(m: FMA) extends MapTester(m, Array(m.io)) {
  val s = m.sigWidth + m.expWidth
  require(s == 32 || s == 64)
  val vars = new collection.mutable.HashMap[Node, Node]()

  def i2d(x: BigInt): Double =
    if (s == 32) java.lang.Float.intBitsToFloat(x.toInt)
    else java.lang.Double.longBitsToDouble(x.toLong)

  def canonicalize(x: BigInt) =
    if (i2d(x).isNaN) (BigInt(1) << s) - 1
    else x

  var ok_me: Boolean = true

  def testOne(s: String) = if (ok_me) {
    val t = s.split(' ')
    vars(c.io.a) = Bits(BigInt(t(0), 16))
    vars(c.io.b) = Bits(BigInt(t(1), 16))
    vars(c.io.c) = Bits(BigInt(t(2), 16))
    vars(c.io.out) = Bits(canonicalize(BigInt(t(3), 16)))
    ok_me = step(vars)
  }

  def testAll: Boolean = {
    val tf_installed = ("which testfloat_gen" ! ProcessLogger(line => ())) == 0
    if (tf_installed) {
      val logger = ProcessLogger(testOne, _ => ())
      Seq("bash", "-c", "testfloat_gen -rnear_even -n 6133248 f"+s+"_mulAdd | head -n10000") ! logger
    } else {
      println("*** FAIL *** install testfloat first! Checkout README for directions.")
      ok_me = false
    }
    ok_me
  }
  defTests {
    testAll
  }
}

object Recoding {
  def recode(in: BigInt, sigWidth: Int, expWidth: Int): BigInt = {
    val sign = x(in, sigWidth+expWidth-1)
    val expIn = x(in, sigWidth+expWidth-2, sigWidth)
    val fractIn = x(in, sigWidth-1, 0)
    val normWidth = 1 << log2Up(sigWidth)

    val isZeroExpIn = expIn == 0
    val isZeroFractIn = fractIn == 0
    val isZero = isZeroExpIn && isZeroFractIn
    val isSubnormal = isZeroExpIn && !isZeroFractIn

    val (norm, normCount) = normalize(fractIn << (normWidth-sigWidth), normWidth)
    val subnormal_expOut = x(-1, expWidth-1, log2Up(sigWidth)) | x(~normCount, log2Up(sigWidth)-1, 0)

    val normalizedFract = x(norm, normWidth-2, normWidth-sigWidth-1)
    val commonExp = if (isZeroExpIn) (if (isZeroFractIn) BigInt(0) else subnormal_expOut) else expIn
    val expAdjust = (if (isZero) 0 else 1 << expWidth-2) | (if (isSubnormal) 2 else 1)
    val adjustedCommonExp = commonExp + expAdjust
    val isNaN = x(adjustedCommonExp, expWidth-1, expWidth-2) == 3 && !isZeroFractIn

    val expOut = x(adjustedCommonExp | ((if (isNaN) 1L else 0L) << (expWidth-3)), expWidth-1, 0)
    val fractOut = if (isZeroExpIn) normalizedFract else fractIn

    (sign << (expWidth+sigWidth)) | (expOut << sigWidth) | fractOut
  }

  def decode(in: BigInt, sigWidth: Int, expWidth: Int): BigInt = {
    val sign = x(in, sigWidth+expWidth)
    val expIn = x(in, sigWidth+expWidth-1, sigWidth)
    val fractIn = x(in, sigWidth-1, 0)

    val isHighSubnormalIn = x(expIn, expWidth-3, 0) < 2
    val isSubnormal = x(expIn, expWidth-1, expWidth-3) == 1 || x(expIn, expWidth-1, expWidth-2) == 1 && isHighSubnormalIn
    val isNormal = x(expIn, expWidth-1, expWidth-2) == 1 && !isHighSubnormalIn || x(expIn, expWidth-1, expWidth-2) == 2
    val isSpecial = x(expIn, expWidth-1, expWidth-2) == 3
    val isNaN = isSpecial && x(expIn, expWidth-3) == 1

    val denormShiftDist = x(2 - x(expIn, log2Up(sigWidth)-1, 0), log2Up(sigWidth)-1, 0).toInt
    val subnormal_fractOut = x(((1L << sigWidth) | fractIn) >> denormShiftDist, sigWidth-1, 0)
    val normal_expOut = x(expIn - ((1 << (expWidth-2))+1), expWidth-2, 0)

    val expOut = if (isNormal) normal_expOut else if (isSpecial) x(-1, expWidth-2, 0) else BigInt(0)
    val fractOut = if (isNormal || isNaN) fractIn else if (isSubnormal) subnormal_fractOut else BigInt(0)

    (sign << (sigWidth+expWidth-1)) | (expOut << sigWidth) | fractOut
  }

  def isNaN(in: BigInt, sigWidth: Int, expWidth: Int): Boolean =
    ((in >> (sigWidth + expWidth - 3)) & 7) == 7

  def canonicalize(in: BigInt, sigWidth: Int, expWidth: Int): BigInt =
    if (!isNaN(in, sigWidth, expWidth)) in
    else in | ((BigInt(1) << sigWidth)-1)

  private def x(n: BigInt, hi: Int, lo: Int): BigInt = (n >> lo) & ((1L << (hi-lo+1))-1)
  private def x(n: BigInt, bit: Int): BigInt = x(n, bit, bit)

  private def normalize(in: BigInt, width: Int) = {
    var log = 0
    for (i <- 1 until width)
      if (((in >> i) & 1) == 1)
        log = i
    val shift = x(~log, log2Up(width)-1, 0).toInt
    (in << shift, shift)
  }
}

object FMATest {
  def main(args: Array[String]): Unit = {
    val testArgs = args.slice(1, args.length)
    args(0) match {
      case "sp-fma" =>
        chiselMainTest(testArgs ++ Array("--compile", "--test", "--genHarness"), () => Module(new FMA(23, 9))) { c => new FMATests(c) }
      case "dp-fma" =>
        chiselMainTest(testArgs ++ Array("--compile", "--test", "--genHarness"), () => Module(new FMA(52, 12))) { c => new FMATests(c) }
    }
  }
}
