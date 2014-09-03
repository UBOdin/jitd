package jitd;

import java.util.*;

import org.apache.logging.log4j.Logger;

public abstract class KeyValueIterator
{
  private static Logger log = 
    org.apache.logging.log4j.LogManager.getLogger();

  public abstract long getKey();
  public abstract long getValue();
  public abstract boolean next();
  
  public static class FilteredIterator extends KeyValueIterator {
    long low, high;
    KeyValueIterator source;
    
    public FilteredIterator(KeyValueIterator source, long low, long high)
    {
      this.source = source;
      this.low = low;
      this.high = high;
    }
    
    public boolean next() 
    { 
      if(!source.next()) { return false; }
      while(source.getKey() < low || source.getKey() >= high) {
        if(!source.next()) { return false; }
      }
      return true;
    }
    
    public long getKey() { return source.getKey(); }
    public long getValue() { return source.getValue(); }
  }
  
  public static class EmptyIterator extends KeyValueIterator {

    public boolean next() { return false; }
    public long getKey() { return 0; }
    public long getValue() { return 0; }
  }
  
  public static class SingletonIterator extends KeyValueIterator {
    boolean nextCalled = false;
    long key, value;
    
    public SingletonIterator(long key, long value){
      this.key = key; this.value = value;
    }
    
    public boolean next() { if(nextCalled) { return false; } nextCalled = true; return true; }
    public long getKey() { return key; }
    public long getValue() { return value; }
  }
  
  public static class SequenceIterator extends KeyValueIterator {
    KeyValueIterator[] seq;
    int i = 0;
    
    public SequenceIterator(KeyValueIterator[] seq){ this.seq = seq; }
    
    public boolean next() {
      while((i < seq.length) && (!seq[i].next())){ i ++; }
      return i < seq.length;
    }
    
    public long getKey(){ return seq[i].getKey(); }
    public long getValue(){ return seq[i].getValue(); }
  }
  
  public static class SubseqIterator extends KeyValueIterator {
    int i;
    KeyValueIterator src;
    
    public SubseqIterator(KeyValueIterator src, int i) 
    {
      this.src = src; this.i = i;
    }
    
    public boolean next(){
      if(i <= 0) { return false; }
      i--;
      return src.next();
    }
    
    public long getKey(){ return src.getKey(); }
    public long getValue(){ return src.getValue(); }
    
  }
  
  public static class RandomIterator extends KeyValueIterator {
    Integer max;
    long key, value;
    Random rand = new Random();
    public static long KEY_RANGE = 10*1000; //1000*1000*1000*1000;
    
    public RandomIterator() { this.max = null; }
    public RandomIterator(int max) { this.max = new Integer(max); }
    
    public static long coerceToKeyRange(long rnd)
    {
      return Math.abs(rnd) % KEY_RANGE;
    }
    
    public long randKey()
    {
      return coerceToKeyRange(rand.nextLong());
    }
    
    public void setSeed(int seed)
    {
      rand = new Random(seed);
    }
    
    public boolean next() { 
      if(max != null){
        if(max <= 0) { return false; }
        max = max - 1;
      }
      key = coerceToKeyRange(rand.nextLong());
      value = rand.nextLong(); 
      return true; 
    }
    public long getKey() { return key; }
    public long getValue() { return value; }
    
  }
  
  /**
   * Basic Merge Join implementation.  Assumes that sources are all sorted.
   * Output is also guaranteed to be sorted.
   */
  public static class MergeIterator extends KeyValueIterator {
  
    KeyValueIterator[] sources;
    int curr;
    
    public MergeIterator(KeyValueIterator[] sources)
    { 
      log.trace("{} created with {} sources");
      this.sources = sources; this.curr = -1;
      for(int i = 0; i < sources.length; i++) {
        if(sources[i] != null){
          if(!sources[i].next()){ sources[i] = null; }
        }
      }
    }
    
    public boolean next()
    {
      log.trace("{} next", this); 
      if(curr >= 0){ 
        if(!sources[curr].next()){ 
          log.trace("{} done with {}", this, curr); 
          sources[curr] = null; 
        }
      }
      curr = -1;
      int i;
      for(i = 0; i < sources.length && sources[i] == null; i++){}
      if(i >= sources.length) { 
        log.trace("{} completely done", this); 
        return false; 
      }
      long currLowest = sources[i].getKey(); 
      curr = i;
      for(i = i + 1; i < sources.length; i++){
        if(sources[i] != null){
          if(sources[i].getKey() < currLowest){
            currLowest = sources[i].getKey();
            curr = i;
          }
        }
      }
      return true;
    }
    
    public long getKey(){ return sources[curr].getKey(); }
    public long getValue(){ return sources[curr].getValue(); }
  }
}