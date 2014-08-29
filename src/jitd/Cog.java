package jitd;

/**
 * The root class of all JIT Datastructure components.  A cog represents a collection 
 * of key/value pairs.  Typically, cogs are not accessed directly, but rather through 
 * subclasses of jitd.Mode.  This allows the implementation of these methods to change 
 * based on how we want the datastructure to behave at this particular instant.
 */


public abstract class Cog 
{
  public abstract KeyValueIterator iterator();
  public abstract int length();
  public abstract long min();
  public abstract long max();
  public String toString(String prefix){ return prefix + toString(); }
}