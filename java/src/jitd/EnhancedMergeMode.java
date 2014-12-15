package jitd;

import java.util.*;

import org.apache.logging.log4j.Logger;

import jitd.util.*;

/**
 *
 */
public class EnhancedMergeMode extends Mode
{

  public static boolean allowInPlace = true;

  private static Logger log = 
    org.apache.logging.log4j.LogManager.getLogger();
  
  public static final int BLOCK_SIZE = 300; //10*1000*1000;
  public static final int MERGE_BLOCK_SIZE = 10; //10000;
  boolean firstRun = true;
  
  public KeyValueIterator scan(Driver driver, long low, long high)
  {
    Pair<KeyValueIterator, Cog> ret = amerge(driver.root, low, high);
    driver.root = ret.b;
    return ret.a;
  }
  
  public Pair<KeyValueIterator, Cog> amerge(Cog cog, long low, long high)
  {
    if(cog instanceof ConcatCog){
      return mergePartitions(cog, low, high);
    } else if(cog instanceof BTreeCog){
      BTreeCog bcog = (BTreeCog)cog;
      Pair<KeyValueIterator, Cog> ret;
      if(high <= bcog.sep){ 
        ret = amerge(bcog.lhs, low, high);
        if(ret.b != bcog.lhs){
          ret.b = new BTreeCog(bcog.sep, ret.b, bcog.rhs);
        } else {
          ret.b = cog;
        }
        return ret;
      } else if(low >= bcog.sep) {
        ret = amerge(bcog.rhs, low, high);
        if(ret.b != bcog.rhs){
          ret.b = new BTreeCog(bcog.sep, bcog.lhs, ret.b);
        } else {
          ret.b = cog;
        }
        return ret;
      } else {
        ret =
          amerge(bcog.lhs, low, bcog.sep);
        Pair<KeyValueIterator, Cog> ret2 =
          amerge(bcog.rhs, bcog.sep, high);
        if(ret.b != bcog.lhs || ret2.b != bcog.rhs){
          ret.b = new BTreeCog(bcog.sep, ret.b, ret2.b);
        } else {
          ret.b = cog;
        }
        ret.a = new KeyValueIterator.SequenceIterator(
          new KeyValueIterator[] { ret.a, ret2.a }
        );
        return ret;
      }
    } else if(cog instanceof ArrayCog && ((ArrayCog)cog).sorted) {
      return new Pair<KeyValueIterator, Cog>(scan(cog, low, high), cog);
    } else if(cog instanceof SubArrayCog && (((SubArrayCog)cog).count == 0
                                             ||((SubArrayCog)cog).base.sorted)) {
      return new Pair<KeyValueIterator, Cog>(scan(cog, low, high), cog);
    } else {
      return amerge(partitionCog(cog), low, high);
    }
  }
  
  public Cog partitionCog(Cog cog){
    long startTime = System.nanoTime();
    // clone the cog, sort it, and fragment into blocks.
    int records = cog.length();

    log.trace("Partitioning Cog: {} ({} records)", cog, records);
    
    if(cog instanceof ArrayCog && allowInPlace && records < BLOCK_SIZE){
      ((ArrayCog)cog).sort();
      log.info("Partition in place: {} us", System.nanoTime() - startTime);
      return cog;
    }
    
    KeyValueIterator source = cog.iterator();
    
    Cog ret = null;
    while(records > 0){
      ArrayCog store = new ArrayCog(Math.min(records, BLOCK_SIZE));
      
      records -= store.length();
      store.load(source);
      store.sort();
      
      if(ret == null){
        ret = store;
      } else {
        ret = new ConcatCog(ret, store);
      }
    }
    if(ret == null){ ret = new SubArrayCog(null, 0, 0); }
    log.trace("Partitioned Cog: {}", ret);
    
    log.info("Partition: {} us ({} records)", cog.length(), System.nanoTime() - startTime);
    return ret;
  }
  
  public Pair<KeyValueIterator, Cog> mergePartitions(Cog cog, long low, long high)
  {
	List<Cog> partitions = gatherPartitions(cog);
    assert(partitions.size() >= 1);

    Cog rightReplacement = new SubArrayCog(null, 0, 0);
    Cog leftReplacement = new SubArrayCog(null, 0, 0);
    KeyValueIterator[] sources = new KeyValueIterator[partitions.size()];
    
    boolean hasMergeWork = false;
    
    int records = 0;
    for(int i = 0; i < partitions.size(); i++){
      log.trace("Partition {}: {} records", i, partitions.get(i).length());
      ExtractedComponents components =
        extractPartitions(partitions.get(i), low, high);
      log.trace("Partition {}: {} lhs, {} rhs, iter: {} in {}-{}", i,
        components.lhs == null ? 0 : components.lhs.length(),
        components.rhs == null ? 0 : components.rhs.length(),
        components.iter == null ? "YES" : "NO", 
        components.lowKey, components.highKey
      );
      
      sources[i] = components.iter;
      if(components.iter != null) { 
        hasMergeWork = true;
      }
      
	  if(components.rhs != null){
		rightReplacement = new ConcatCog(rightReplacement, components.rhs);
		records += components.rhs.length();
	  }
	  if(components.lhs != null){
		leftReplacement = new ConcatCog(leftReplacement, components.lhs);
        records += components.lhs.length();
	  }
    }
    log.trace("Records Retained: {}", records);
    
    if(hasMergeWork){
      log.trace("Root: {} records", partitions.get(0).length());
      long startTime = System.nanoTime();

      // Merge join
      KeyValueIterator mergedStream = 
        new KeyValueIterator.MergeIterator(sources);
      
      BTreeCog.FoldAccum fold = new BTreeCog.FoldAccum();
      // Build a new sequence of blocks out of the constructed values.
      records = 0;
      
      while(true){
        long startMergeTime = System.nanoTime();
        ArrayCog buffer = new ArrayCog(MERGE_BLOCK_SIZE);
        buffer.sorted = true;
        int len = buffer.load(mergedStream);
        if(len == 0){ break; }
        fold.append(buffer, buffer.min());
        long endMergeTime = System.nanoTime();
        log.info("Create Merge Block: {} us", (endMergeTime-startMergeTime)/1000);
        records += buffer.length();
        if(len < MERGE_BLOCK_SIZE){ break; }
      }
      log.info("Merge Join: {} us ({} records merged)", System.nanoTime() - startTime, records);
      
      Cog rootMID = fold.fold();
      log.trace("Merge BTree {}", rootMID);
      
      KeyValueIterator ret;
      
      if(rootMID == null){
        ret = new KeyValueIterator.EmptyIterator();
        rootMID = new BTreeCog(high, leftReplacement, rightReplacement);
      } else {
        ret = scan(rootMID, low, high);
        if(leftReplacement.length() != 0){
          rootMID = new BTreeCog(rootMID.min(), leftReplacement, rootMID);
        }
        if(rightReplacement.length() != 0){
          rootMID = new BTreeCog(rightReplacement.min(), rootMID, rightReplacement);
        }
      }
      return new Pair<KeyValueIterator, Cog>(ret, rootMID);
    } else {
      long startTime = System.nanoTime();
      Pair<KeyValueIterator, Cog> ret = amerge(partitions.get(0), low, high);
      if(leftReplacement != null){
        ret.b = new ConcatCog(ret.b, leftReplacement);
      }
      if(rightReplacement != null){
        ret.b = new ConcatCog(ret.b, rightReplacement);
      }
      log.info("Reassemble: {} us", System.nanoTime() - startTime);
      return ret;
    }
  }
  
  public ExtractedComponents extractPartitions(Cog cog, long low, long high)
  {
    return extractPartitions(cog, low, high, false);
  }
  public ExtractedComponents extractPartitions(Cog cog, long low, long high, boolean fullBlocks)
  {
    assert(!(cog instanceof ConcatCog));
    cog = amerge(cog, low, high).b;
    
    if(cog instanceof BTreeCog){
      BTreeCog bcog = (BTreeCog)cog;
      ExtractedComponents ret;
      if(bcog.sep <= low){
        ret = extractPartitions(bcog.rhs, low, high, fullBlocks);
        ret.lhs = (ret.lhs == null) ? bcog.lhs : new BTreeCog(bcog.sep, bcog.lhs, ret.lhs);
      } else if(bcog.sep >= high){
        ret = extractPartitions(bcog.lhs, low, high, fullBlocks);
        ret.rhs = (ret.rhs == null) ? bcog.rhs : new BTreeCog(bcog.sep, ret.rhs, bcog.rhs);
      } else {
        ret = extractPartitions(bcog.lhs, low, bcog.sep, fullBlocks);
        ExtractedComponents ret2 =
              extractPartitions(bcog.rhs, bcog.sep, high, fullBlocks);
        // since the separator is between low & high, the LHS by definition cannot 
        // produce a chunk > high.  Similarly, the rhs cannot produce a chunk < low
        assert(ret.rhs == null);
        assert(ret2.lhs == null);
        ret.rhs = ret2.rhs;
        
        // Both, however, may still produce iterators.
        if(ret.iter == null){
          ret.iter = ret2.iter;
          ret.lowKey = ret2.lowKey;
          ret.highKey = ret2.highKey;
        } else if(ret2.iter != null) {
          ret.iter = new KeyValueIterator.SequenceIterator(
            new KeyValueIterator[] { ret.iter, ret2.iter }
          );
          ret.lowKey = Math.min(ret.lowKey, ret2.lowKey);
          ret.highKey = Math.max(ret.highKey, ret2.highKey);
        }
      }
      return ret;
    } else if(cog instanceof ArrayCog && ((ArrayCog)cog).sorted) {
      if(fullBlocks) { 
        return new ExtractedComponents(null, null, cog.min(), cog.max(), cog.iterator()); 
      }
      ArrayCog acog = (ArrayCog)cog;
      int lowIdx = acog.indexOf(low);
      int highIdx = acog.indexOfFirst(high);
      
      Cog lhs = lowIdx > 0 ? 
        new SubArrayCog(acog, 0, lowIdx) : null;
      Cog rhs = highIdx < acog.length() ? 
        new SubArrayCog(acog, highIdx, acog.length() - highIdx) : null;
      KeyValueIterator iter = lowIdx < highIdx ? acog.subseqIterator(lowIdx, highIdx) : null;
      long lowKey = (iter == null) ? Long.MAX_VALUE : acog.keys[lowIdx];
      long highKey = (iter == null) ? Long.MIN_VALUE : acog.keys[highIdx-1];
      
      return new ExtractedComponents(lhs, rhs, lowKey, highKey, iter);
      
    } else if(cog instanceof SubArrayCog && (((SubArrayCog)cog).count <= 1 ||
                                             ((SubArrayCog)cog).base.sorted)) {
      if(fullBlocks) { 
        return new ExtractedComponents(null, null, cog.min(), cog.max(), cog.iterator()); 
      }
      SubArrayCog scog = (SubArrayCog)cog;
      if(scog.count == 0){
        return new ExtractedComponents(null, null, Long.MAX_VALUE, Long.MIN_VALUE, null);
      }
      
      int lowIdx = 
        Math.min(scog.start+scog.count,
                 Math.max(scog.base.indexOf(low), scog.start));
      int highIdx = 
        Math.min(scog.start+scog.count,
                 Math.max(scog.base.indexOfFirst(high), scog.start));
      
      Cog lhs = lowIdx > scog.start ? 
        new SubArrayCog(scog.base, scog.start, lowIdx-scog.start) : null;
      Cog rhs = highIdx < scog.start+scog.count ? 
        new SubArrayCog(scog.base, highIdx, scog.start+scog.count-highIdx) : null;

      KeyValueIterator iter = lowIdx < highIdx ? scog.base.subseqIterator(lowIdx, highIdx) : null;
      long lowKey = (iter == null) ? Long.MAX_VALUE : scog.base.keys[lowIdx];
      long highKey = (iter == null) ? Long.MIN_VALUE : scog.base.keys[highIdx-1];
      return new ExtractedComponents(lhs, rhs, lowKey, highKey, iter);
    } else {
      assert(false);
      return null;
    }
  }
  
  protected List<Cog> gatherPartitions(List<Cog> ret, Cog curr)
  {
    if(curr instanceof ConcatCog){
      ConcatCog ccog = (ConcatCog)curr;
      gatherPartitions(ret, ccog.lhs);
      gatherPartitions(ret, ccog.rhs);
    } else {
    	if(curr instanceof ArrayCog && !((ArrayCog)curr).sorted){
  	  gatherPartitions(ret, partitionCog(curr));
    } else {
      ret.add(curr);
    }
    }
    return ret;
  }
  public List<Cog> gatherPartitions(Cog root)
  {
    return gatherPartitions(new ArrayList<Cog>(20), root);
  }
  
  public static class ExtractedComponents
  {
    public Cog lhs, rhs;
    public long lowKey, highKey;
    public KeyValueIterator iter;
    
    public ExtractedComponents(Cog lhs, Cog rhs, long lowKey, long highKey, KeyValueIterator iter)
    {
      this.lhs = lhs;
      this.rhs = rhs;
      this.lowKey = lowKey;
      this.highKey = highKey;
      this.iter = iter;
    }
  }
  
  
}