package jitd;

/**
 * A cog representing a fragment of an Array Cog
 */

import java.util.*; 

public class SubArrayCog extends Cog
{
  ArrayCog base;
  int start, count;

  public SubArrayCog(ArrayCog base, int start, int count){
    if(count == 0){ this.base = null; }
    else { this.base = base; }
    if(count < 0) {
      throw new UnsupportedOperationException();
    }
    this.start = start; this.count = count;
  }
  
  public KeyValueIterator iterator()
  {
    if(count == 0){ return new KeyValueIterator.EmptyIterator(); } 
    return base.subseqIterator(start, start+count);
  }
  
  public int length()
  {
    return count;
  }
  
  public String toString()
  {
    return "SubArray("+start+", "+count+", "+base+")";
  }
  
  public long min()
  {
    if(base == null){ return Long.MAX_VALUE; }
    return base.min();
  }
  public long max()
  {
    if(base == null){ return Long.MIN_VALUE; }
    return base.max();
  }

}