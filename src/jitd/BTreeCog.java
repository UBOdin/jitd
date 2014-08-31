package jitd;

/**
 * A cog representing a B-Tree Node
 * 
 * BTree(sep, lhs, rhs) indicates that lhs and rhs have been radix partitioned
 * by the value 'sep'.  'lhs' is guaranteed to contain only keys strictly lower than 
 * 'sep', while 'rhs' is guaranteed to contain only keys greater than or equal to 'sep'.
 */

import java.util.*;
import jitd.util.*;

public class BTreeCog extends Cog
{
  public long sep;
  public Cog lhs, rhs;
  
  public BTreeCog(long sep, Cog lhs, Cog rhs) {
    if(lhs == null || rhs == null) {
      throw new UnsupportedOperationException();
    }
    this.sep = sep; this.lhs = lhs; this.rhs = rhs;
  }
  
  public KeyValueIterator iterator()
  {
    return new KeyValueIterator.SequenceIterator(
      new KeyValueIterator[] {
        lhs.iterator(), rhs.iterator()
      }
    );
  }
  
  public int length()
  {
    return lhs.length() + rhs.length();
  }
  
  public String toString() { return toString(""); }
  
  public String toString(String prefix)
  {
    return prefix + "BNode("+sep+"\n"+
            lhs.toString(prefix+"  ")+", \n"+
            rhs.toString(prefix+"  ")+", \n"+
           prefix + ")";
  }
  
  public String toLocalString()
  {
    return "BNode("+sep+")";
  }
  
  public long min(){
    long ret = lhs.min();
    if(ret == Long.MAX_VALUE){ return rhs.min(); }
    return ret;
  }
  
  public long max(){
    long ret = rhs.max();
    if(ret == Long.MIN_VALUE){ return lhs.max(); }
    return ret;
  }
  
  public static class FoldAccum {
    Stack<Triple<Cog, Integer, Long>> stack = new Stack<Triple<Cog, Integer, Long>>();

    public FoldAccum() {}
    
    public void append(Cog next, long low)
    {
      int depth = 0;
      while(!stack.empty() && (depth == stack.peek().b)){
        Triple<Cog, Integer, Long> head = stack.pop();
        next = new BTreeCog(low, head.a, next);
        low = head.c;
        depth++;
      }
      stack.push(new Triple<Cog, Integer, Long>(next, depth, low));
    }
    
    public Cog fold()
    {
      if(stack.empty()) { return null; }
      Triple<Cog, Integer, Long> head = stack.pop();
      while(!stack.empty()){
        Triple<Cog, Integer, Long> prev = stack.pop();
        head.a = new BTreeCog(head.c, prev.a, head.a);
        head.c = prev.c;
      }
      return head.a;
    }
  }
  
  public List<Cog> children() { 
    return Arrays.asList(new Cog[]{ lhs, rhs }); 
  }
}