package jitd;

import java.util.*;

/**
 * A cog representing a collection of objects.  Similar to ArrayCog, except not fixed-size.  
 */

import java.util.*; 

public class BufferCog extends Cog
{
  public static final int BLOCK_SIZE = 1024;

  LinkedList<Block> blocks;

  public BufferCog() { 
    this.blocks = new LinkedList<Block>();
    this.blocks.add(new Block());
  }
  
  public int length()
  {
    int fill = 0;
    for(Block b : blocks) { fill += b.fill; }
    return fill;
  }

  public long min()
  {
    long ret = Long.MAX_VALUE;
    for(Block block : blocks){
      for(int i = 0; i < block.fill; i++){
        long k = block.keys[i];
        ret = k < ret ? k : ret; 
      }
    }
    return ret;
  }
  public long max()
  {
    long ret = Long.MIN_VALUE;
    for(Block block : blocks){
      for(int i = 0; i < block.fill; i++){
        long k = block.keys[i];
        ret = k > ret ? k : ret; 
      }
    }
    return ret;
  }  
  public void append(long key, long value)
  {
    Block last = this.blocks.getLast();
    if(last.fill >= last.keys.length){
      last = new Block();
      blocks.add(last);
    }
    last.keys[last.fill] = key;
    last.values[last.fill] = value;
    last.fill++;
  }
  
  public KeyValueIterator iterator() {
    return new BufferIterator();
  }
  
  public class BufferIterator extends KeyValueIterator {
    Iterator<Block> iter = blocks.listIterator();
    Block curr = null;
    int i = 0;
    
    public boolean next()
    {
      if(iter == null){ return false; }
      while((curr == null) || (i < curr.fill)){
        if(!iter.hasNext()) { iter = null; return false; }
        curr = iter.next();
        i = 0;
      }
      i++;
      return true;
    }
    public long getKey() { return curr.keys[i]; }
    public long getValue() { return curr.values[i]; }
  }
  
  protected static class Block {
    public long keys[];
    public long values[];
    public int fill;
    
    public Block() { 
      this.keys = new long[BLOCK_SIZE];
      this.values = new long[BLOCK_SIZE];
      this.fill = 0;
    }
  }
  
  public String toString()
  {
    return "Buffer(...["+length()+" records]...)";
  }
  
}