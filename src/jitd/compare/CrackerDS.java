package jitd.compare;

import java.util.*;

public class CrackerDS implements RangeDS {
  
  long keys[];
  long values[];
  IndexNode root;
  
  public CrackerDS(int size)
  {
    keys = new long[size];
    values = new long[size];
    root = new LeafNode(0, size);
  }
  
  public void loadRandom()
  {
    Random rand = new Random();
    int step = keys.length / 100;
    for(int i = 0; i < keys.length; i++){
      if(i % step == 0){
        System.out.println(""+(i/step)+"% complete");
      }
      keys[i] = rand.nextLong();
      values[i] = rand.nextLong();
    }
  }
  
  public int fragment(long radix)
  {
    LeafNode leaf = root.get(radix);
    
//    System.out.println("Fragment: " + radix);
    
    if(leaf.low != null  && leaf.low >= radix ){ return leaf.start; }
    if(leaf.high != null && leaf.high <= radix){ return leaf.start+leaf.count; }
    
//    System.out.println("  populated: "+leaf.start+" -- "+(leaf.start+leaf.count));
    
    long low = Long.MAX_VALUE, high = Long.MIN_VALUE;
    long tmp;
    int radixPos = 0, i;
    
    
    for(i = 0; i < leaf.count; i++){
      int curr = i + leaf.start;
      long key = keys[curr];
      low = Math.min(low, key);
      high = Math.max(high, key);
//      System.out.println(""+key+" < "+radix);
      if(key < radix){
//        System.out.println("  swap!");
        if(i != radixPos){
          int swapPos = radixPos+leaf.start;
          tmp = keys[curr];
          keys[curr] = keys[swapPos];
          keys[swapPos] = tmp;
          tmp = values[curr];
          values[curr] = values[swapPos];
          values[swapPos] = tmp;
        }
        radixPos ++;
      }
    }
    
    if(radixPos == 0){
      leaf.low = low;
      return leaf.start;
    }
    if(radixPos >= leaf.count){
      leaf.high = high;
      return leaf.start + leaf.count;
    }

    LeafNode newLhs = new LeafNode(leaf.start, radixPos);
    newLhs.low = low; newLhs.high = radix;

    LeafNode newRhs = new LeafNode(leaf.start+radixPos, leaf.count-radixPos);
    newRhs.low = radix; newRhs.high = high;
    
    BranchNode subNode = new BranchNode(newLhs, radix, newRhs);
    newLhs.parent = subNode;
    newRhs.parent = subNode;
    
    if(leaf.parent == null){
      root = subNode;
    } else if(leaf.parent.lhs == leaf){
      leaf.parent.lhs = subNode;
    } else {
      leaf.parent.rhs = subNode;
    }
    
    return leaf.start + radixPos;
  }
  
  public void insert(long k, long v)
  {
    System.out.println("Error: Insert unsupported");
    System.exit(-1);
  }
  public void delete(long k)
  {
    System.out.println("Error: Delete unsupported");
    System.exit(-1);
  }
  
  public long lookup(long k)
  {
    System.out.println("Error: Lookup unsupported");
    System.exit(-1);
    return 0;
  }
  
  public RecordIterator scan(long low, long high)
  {
    int start = fragment(low);
    int end = fragment(high);
    return new Range(start, end-start);
  }
  
  protected static interface IndexNode { 
    public LeafNode get(long radix);
  }
  
  protected static class BranchNode implements IndexNode { 
    public IndexNode lhs, rhs;
    public long radix;
    
    public BranchNode(IndexNode lhs, long radix, IndexNode rhs)
    {
      this.lhs = lhs; this.rhs = rhs; this.radix = radix;
    }
    
    public LeafNode get(long radix){
      if(radix < this.radix) { return lhs.get(radix); }
      else { return rhs.get(radix); }
    }
  }
  
  protected static class LeafNode implements IndexNode {
    public int start, count;
    public Long low = null, high = null;
    public BranchNode parent = null;
    
    public LeafNode(int start, int count)
    {
      this.start = start; this.count = count;
    }
    
    public LeafNode get(long radix)
    {
      return this;
    }
  }
  
  public class Range implements RecordIterator {
    public  int start, length;
    protected int idx = -1;
    public Range(int start, int length) { this.start = start; this.length = length; }
    
    public boolean next() { if(idx < length-1) { idx++; return true; } else { return false; } }
    public long getKey() { return keys[start+idx]; }
    public long getValue() { return values[start+idx]; }
  }
}