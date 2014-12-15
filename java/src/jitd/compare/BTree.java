package jitd.compare;

import java.util.*;
import jitd.compare.RangeDS.RecordIterator;
import jitd.compare.RangeDS.RecordRand;
import jitd.compare.RangeDS.RecordMerge;


public class BTree {
  
  public static final int RECORDS_PER_BLOCK = 32 * 1024;
  
  public Block firstData, lastData;
  public Node root;
  
  public BTree()
  {
    firstData = lastData = null;
    root = null;
  }
  
  public void loadRandom(int size)
  {
    firstData = Block.toChain(new RecordRand(size));
    lastData = firstData.last();
  }
  
  public void buildSort()
  {
    firstData = firstData.sortChain();
    lastData = firstData.last();
  }
  
  public void buildTree()
  {
    Stack<NodeStack> stack = new Stack<NodeStack>();
    Block curr = firstData;
    Node root = null;
    long low = 0;
    
    while(curr != null){
      root = curr;
      int level = 0;
      low = curr.keys[0];
      while(stack.peek().level == level){
        NodeStack lhsNode = stack.pop();
        root =
          new InnerNode(
            low, 
            lhsNode.node,
            root
          );
        low = lhsNode.low;
        level++;
      }
      curr = curr.next;
    }
    if(root != null){
      while(!stack.empty()){
        NodeStack lhsNode = stack.pop();
        root =
          new InnerNode(
            low, 
            lhsNode.node,
            root
          );
        low = lhsNode.low;
      }
    }
  }
  
  public static class NodeStack
  {
    public Node node;
    public int level;
    public long low;
    public NodeStack(Node node, int level){ this.node = node; this.level = level; }
  }
  
  public static class Block implements Node
  {
    public Block next, prev;
    public int fill;
    long keys[];
    long values[];
    
    public Block(){
      fill = 0;
      next = prev = null;
      keys = new long[RECORDS_PER_BLOCK];
      values = new long[RECORDS_PER_BLOCK];
    }
    
    public Block(RangeDS.RecordIterator iter){
      this();
      while(iter.next() && fill < RECORDS_PER_BLOCK){
        keys[fill] = iter.getKey();
        values[fill] = iter.getValue();
        fill++;
      }
    }
    
    public void swap(int i, int j)
    {
      long tmp;
      tmp = keys[i];
      keys[i] = keys[j];
      keys[j] = tmp;
      tmp = values[i];
      values[i] = values[j];
      values[j] = tmp;
      
    }
    
    //quicksort
    protected void sort(int start, int count){
      if(count <= 1) { return; }
      if(count == 2) {
        if(keys[start] > keys[start+1]){ swap(start, start+1); }
      }
      long radix = keys[start];
      int i, radixPos;

      // partition
      for(i = 1, radixPos = 0; i < count; i++){
        if(keys[i] < radix){ swap(start+i, start+radixPos); radixPos++; }
      }

      // recur
      if(radixPos > 0){
        // there are some records < radix
        sort(0, radixPos);
      } else if(radixPos < count-1) {
        // there are some non-radix records >= radix
        sort(radixPos+1, count-1);
      }
    }
    
    public void sort(){ sort(0, keys.length); }
    
    public int length(){
      return next == null ? 1 : next.length()+1;
    }
    
    public Block last(){
      if(next == null) { return this; }
      return next.last();
    }
    
    public RecordIterator iter() { 
      return new RecordIterator(){
        int curr = -1;
        int last = keys.length-1;
        public boolean next()
        { 
          if(curr < last) { curr++; return true; } else { return false; } 
        }
        public long getKey() { return keys[curr]; }
        public long getValue() { return values[curr]; }
      };
    }
    
    public Block sortChain(){
      RecordIterator iters[] = new RecordIterator[length()];
      Block curr = this;
      int i = 0;
      while(curr != null){
        iters[i] = curr.iter();
        curr = curr.next; i++;
      }
      return toChain(new RecordMerge(iters)); 
    }
    
    public static Block toChain(RecordIterator iter){
      Block first, curr;
      curr = first = new Block(iter);
      if(curr.fill > 0){
        curr.next = new Block(iter);
        while(curr.next.fill > 0){
          curr.next.prev = curr;
          curr = curr.next;
          curr.next = new Block(iter);
        }
        curr.next = null;
      }
      return first;
    }
  }
  
  public interface Node 
  {
    
  }
  
  public class InnerNode implements Node
  {
    public Node lhs, rhs;
    public long sep;
    public InnerNode(long sep, Node lhs, Node rhs)
      { this.sep = sep; this.lhs = lhs; this.rhs = rhs; }
  }
}