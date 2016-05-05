/*
 Copyright (c) 2011, 2012, 2013, 2014 The Regents of the University of
 California (Regents). All Rights Reserved.  Redistribution and use in
 source and binary forms, with or without modification, are permitted
 provided that the following conditions are met:

    * Redistributions of source code must retain the above
      copyright notice, this list of conditions and the following
      two paragraphs of disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      two paragraphs of disclaimer in the documentation and/or other materials
      provided with the distribution.
    * Neither the name of the Regents nor the names of its contributors
      may be used to endorse or promote products derived from this
      software without specific prior written permission.

 IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
 SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS,
 ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF
 REGENTS HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
 LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF
 ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". REGENTS HAS NO OBLIGATION
 TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
 MODIFICATIONS.
*/

package Chisel

import Node._
import ChiselError._

abstract trait Num[T <: Data] {
  // def << (b: T): T;
  // def >> (b: T): T;
  def unary_-(): T;
  def +  (b: T): T;
  def *  (b: T): T;
  def /  (b: T): T;
  def %  (b: T): T;
  def -  (b: T): T;
  def <  (b: T): Bool;
  def <= (b: T): Bool;
  def >  (b: T): Bool;
  def >= (b: T): Bool;

  def min(b: T): T = Mux(this < b, this.asInstanceOf[T], b)
  def max(b: T): T = Mux(this < b, b, this.asInstanceOf[T])
}

/** *Data* is part of the *Node* Composite Pattern class hierarchy.
  It is the root of the type system which includes composites (Bundle, Vec)
  and atomic types (UInt, SInt, etc.).

  Instances of Data are meant to help with construction and correctness
  of a logic graph. They will trimmed out of the graph before a *Backend*
  generates target code.
  */
abstract class Data extends Node {
  var comp: proc = null;

  // Interface required by Vec:
  def ===[T <: Data](right: T): Bool = {
    throw new Exception("=== not defined on " + this.getClass
      + " and " + right.getClass)
  }

  def toBool(): Bool = {
    val gotWidth = this.getWidth()
    if( gotWidth < 1) {
      throw new Exception("unable to automatically convert " + this + " to Bool, convert manually instead")
    } else if(gotWidth > 1) {
      throw new Exception("multi bit signal " + this + " converted to Bool");
    }
    chiselCast(this){Bool()};
  }

  // Interface required by Cat:
  def ##[T <: Data](right: T): this.type = {
    throw new Exception("## not defined on " + this.getClass + " and " + right.getClass)
  }


  def setIsTypeNode {
    assert(inputs.length > 0, ChiselError.error("Type Node must have an input"))
    isTypeNode = true
    inferWidth = widthOf(0)
  }

  def apply(name: String): Data = null
  def flatten: Array[(String, Bits)]
  def flip(): this.type = this;
  def asInput(): this.type = this;

  /** Sets the direction (*dir*) of instances derived from Bits to OUTPUT
    or recursively sets members of Bundle/Vec to OUTPUT.
    Returns this instance with its exact type.
    */
  def asOutput(): this.type
  def asDirectionless(): this.type
  def isDirectionless: Boolean = true;

  /** Factory method to create and assign a leaf-type instance out of a subclass
    of *Node* instance which we have lost the concrete type. */
  def fromNode(n: Node): this.type = {
    val res = this.clone
    val packet = res.flatten.reverse.zip(this.flatten.reverse.map(_._2.getWidth))
    var ind = 0
    for (((name, io), gotWidth) <- packet) {
      io.asOutput()
      val assignWidth = if (gotWidth > 0) ind + gotWidth - 1 else -1
      io assign NodeExtract(n, assignWidth, ind)
      ind += (if (gotWidth > 0) gotWidth else 0)
    }
    res.setIsTypeNode
    res
  }

  def fromBits(b: Bits): this.type = this.fromNode(b)

  override lazy val toNode: Node = {
    val nodes = this.flatten.map(_._2)
    Concatenate(nodes.head, nodes.tail:_*)
  }

  def :=(that: Data): Unit = that match {
    case b: Bits => this colonEquals b
    case b: Bundle => this colonEquals b
    case b: Vec[_] => this colonEquals b
    case _ => illegalAssignment(that)
  }

  protected def colonEquals(that: Bits): Unit = illegalAssignment(that)
  protected def colonEquals(that: Bundle): Unit = illegalAssignment(that)
  protected def colonEquals[T <: Data](that: Iterable[T]): Unit = illegalAssignment(that)

  protected def illegalAssignment(that: Any): Unit =
    ChiselError.error(":= not defined on " + this.getClass + " and " + that.getClass)

  override def clone(): this.type = {
    try {
      val constructor = this.getClass.getConstructors.head

      if(constructor.getParameterTypes.size == 0) {
        constructor.newInstance().asInstanceOf[this.type]
      } else if(constructor.getParameterTypes.size == 1) {
        val paramtype = constructor.getParameterTypes.head
        // If only 1 arg and is a Bundle or Module then this is probably the implicit argument
        //    added by scalac for nested classes and closures. Thus, try faking the constructor
        //    by not supplying said class or closure (pass null).
        // CONSIDER: Don't try to create this
        if(classOf[Bundle].isAssignableFrom(paramtype) || classOf[Module].isAssignableFrom(paramtype)){
          constructor.newInstance(null).asInstanceOf[this.type]
        } else throw new Exception("Cannot auto-create constructor for ${this.getClass.getName} that requires arguments")
      } else {
       throw new Exception(s"Cannot auto-create constructor for ${this.getClass.getName} that requires arguments")
      }

    } catch {
      case npe: java.lang.reflect.InvocationTargetException if npe.getCause.isInstanceOf[java.lang.NullPointerException] =>
        throwException("Parameterized Bundle " + this.getClass + " needs clone method. You are probably using an anonymous Bundle object that captures external state and hence is un-cloneable", npe)
      case e: java.lang.Exception =>
        throwException("Parameterized Bundle " + this.getClass + " needs clone method", e)
    }
  }

  override def nameIt(path: String, isNamingIo: Boolean) {
    if (isTypeNode && comp != null) {
      comp.nameIt(path, isNamingIo)
    } else {
      super.nameIt(path, isNamingIo)
    }
  }

  def params = if(Driver.parStack.isEmpty) Parameters.empty else Driver.parStack.top
}
