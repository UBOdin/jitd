package jitd;

/**
 * A cog representing a single k/v pair.
 * 
 * Leaf(k, v) is a singleton collection.
 */

import java.util.*; 

public class LeafCog extends Cog
{
  public long key, value;
  
  public LeafCog(long key, long value) { this.key = key; this.value = value; }
  
  public KeyValueIterator iterator()
  {
    return new KeyValueIterator.SingletonIterator(key, value); 
  }

  public int length()
  {
    return 1;
  }
  
  public String toString()
  {
    return "Leaf("+key+")";
  }

  public long min()
  {
    return key;
  }
  public long max()
  {
    return key;
  }
}