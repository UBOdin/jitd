package jitd;

/**
 * A cog representing an unordered concatenation of two input Cogs. 
 * 
 * Concat(lhs, rhs) contains all of the k/v pairs in both lhs and rhs.
 */

import java.util.*; 

public class ConcatCog extends Cog
{
  public Cog lhs, rhs;
  
  public ConcatCog(Cog lhs, Cog rhs) { 
    if(lhs == null || rhs == null) {
      throw new UnsupportedOperationException();
    }
    this.lhs = lhs; this.rhs = rhs; 
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
    return prefix + "Concat(\n"+
            lhs.toString(prefix+"  ")+", \n"+
            rhs.toString(prefix+"  ")+", \n"+
           prefix + ")";
  }
  
  public String toLocalString()
  {
    return "Concat(...)";
  }

  public long min()
  {
    return Math.min(lhs.min(), rhs.min());
  }
  public long max()
  {
    return Math.max(lhs.max(), rhs.max());
  }

  public List<Cog> children() { 
    return Arrays.asList(new Cog[]{ lhs, rhs }); 
  }
}