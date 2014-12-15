package jitd.test;

import java.util.*;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import org.junit.*;
import org.junit.runner.*;
import org.junit.runners.*;
import org.junit.runners.Parameterized.Parameters;

import jitd.*;

@RunWith(Parameterized.class)
public class ModeTest extends CogTest {
  
  @Parameters(name="{1}")
  public static Collection<Object[]> data() {
    return Arrays.asList(new Object[][] {
      { new Mode(), "Default Mode" },
      { new CrackerMode(), "Cracker Mode" },
      { new MergeMode(), "Merge Mode" },
      { new PushdownMergeMode(), "Pushdown Merge Mode" },
      { new EnhancedMergeMode(), "Enhanced Merge Mode" }
    });
  }
  
    
  Driver driver;
  String testName = "[UNKNOWN]";
  String trialName;
  
  public ModeTest(Mode mode, String trial)
  {
    driver = new Driver(mode, null);
    trialName = trial;
  }
  
  Set<Long> expected = new TreeSet<Long>();
  
  public void testRead(String trial)
  {
    testRead(trial, 100, 200);
  }
  
  public void testRead(String trial, long low, long high)
  {
    KeyValueIterator ret = driver.scan(low, high);
    testRead(trial, ret);
  }
  
  public void testRead(String trial, KeyValueIterator src)
  {
    Set<Long> keysFound = new TreeSet<Long>();
    
    while(src.next()){
      keysFound.add(src.getKey());
    }
    
    if(!expected.equals(keysFound)){
      dump();
      assertEquals(trialName+"-"+testName + " ["+trial+"]", expected, keysFound);
    }
  }
  
  public void load(Cog c)
  {
    driver.root = c;
    resetExpected();
  }
  
  public void dump()
  {
    System.out.println(trialName+"-"+testName+" [STATE]: \n" + driver.root.toString("   "));
  }
  
  public void insert(Cog c)
  {
    driver.insert(c);
  }
  
  public void resetExpected()
  {
    expected = new TreeSet<Long>();
  }
  
  public void expect(long[] ks)
  {
    expect(ks, 100, 200);
  }

  public void expect(long[] ks, long low, long high)
  {
    for(long k : ks){ 
      if(k >= low && k < high){
        expected.add(k); 
      }
    }
  }
  
  public void expect(long l)
  {
    expect(new long[] { l });
  }
  
  @Test
  public void testStructureReads() 
  {
    testName = "Read";
    
    load(array(test1));
    expect(test1);
    testRead("Array1");
    
    load(array(test2));
    expect(test2);
    testRead("Array2");
    
    for(int size : new int[] { 10, 100, 1000 }){
      long[] data = randArray(size);

      load(array(data));
      expect(data);
      testRead("Rand Array ("+size+")");
    }
    
    load(leaf(50));
    //empty expect
    testRead("Leaf-50");

    load(leaf(99));
    //empty expect
    testRead("Leaf-99");

    load(leaf(100));
    expect(100);
    testRead("Leaf-100");

    load(leaf(101));
    expect(101);
    testRead("Leaf-101");
    
    load(leaf(150));
    expect(150);
    testRead("Leaf-150");

    load(leaf(199));
    expect(199);
    testRead("Leaf-199");

    load(leaf(200));
    //empty expect
    testRead("Leaf-200");

    load(leaf(201));
    //empty expect
    testRead("Leaf-201");

    load(leaf(250));
    //empty expect
    testRead("Leaf-250");
    
    load(concat(array(test1), array(test2)));
    expect(test1); expect(test2);
    testRead("Concat1");

    load(concat(array(test1), leaf(50)));
    expect(test1); 
    testRead("Concat2");

    load(concat(array(test1), leaf(150)));
    expect(test1); expect(150); 
    testRead("Concat3");
    
    load(bnode(400, array(test1), leaf(450)));
    expect(test1);
    testRead("BNode1");

    load(bnode(0, leaf(-50), array(test1)));
    expect(test1);
    testRead("BNode2");
    
    load(bnode(400, leaf(120), leaf(160)));
    expect(120); // The 160 violates semantics.  We shouldn't see it since [100,200) < 400
    testRead("BNode3");
    
    load(bnode(150, leaf(120), leaf(160)));
    expect(120); expect(160); //150 \in [100,200)
    testRead("BNode4");
    
    load(bnode(199, leaf(120), leaf(199)));
    expect(120); expect(199); 
    testRead("BNode5");

    load(bnode(200, leaf(120), leaf(200)));
    expect(120); 
    testRead("BNode6");

    load(bnode(200, leaf(120), leaf(199)));
    expect(120); // 200 > [100,200), so even if the RHS is in bounds, we shouldn't see it.
    testRead("BNode7");
    
    load(bnode(101, leaf(100), leaf(160)));
    expect(100); expect(160); 
    testRead("BNode8");

    load(bnode(100, leaf(99), leaf(160)));
    expect(160); 
    testRead("BNode9");

    load(bnode(100, leaf(120), leaf(160)));
    expect(160); // As above, the LHS of a BTree node should be exclusively values lower 
                 // than the separator, so we should never see the 120
    testRead("BNode10");
    
    
    load(sorted(test1));
    expect(test1);
    testRead("Sorted1");

    load(sorted(test2));
    expect(test2);
    testRead("Sorted2");
    
    load(sorted(test3));
    expect(test3);
    testRead("Sorted3");

    for(int size : new int[] { 10, 10, 10, 10, 10, 10, 100, 100, 100, 1000 }){
      long[] data = randArray(size);

      load(sorted(data));
      expect(data);
      testRead("Rand Array ("+Arrays.toString(data)+")");
    }
  }
  
  
  @Test @Ignore
  public void testStructureWrites()
  {
    testName = "Write";
    
    load(array(test1));
    insert(leaf(150));
    expect(150); expect(test1);
    testRead("Append leaf to array");
//    dump();

    load(leaf(150));
    insert(array(test1));
    expect(150); expect(test1);
    testRead("Append array to leaf");
//    dump();
    
    load(array(test1));
    insert(array(test2));
    expect(test1); expect(test2);
    testRead("Append array to array");
//    dump();
    
    insert(array(test3));
    expect(test3);
    testRead("Append another array");
//    dump();
  }
  
  @Test
  public void testStructureReadIter()
  {
    testName = "ReadIter";
    
    load(array(test1));
    for(long[] arg : new long[][] {
      { 100, 200 },
      { 90, 100 },
      { 95, 150 },
      { 180, 250 }
    }) {
      resetExpected();
      expect(test1, arg[0], arg[1]);
//      System.out.println("Preparing to read: "+arg[0]+"-"+arg[1]);
//      dump();
      testRead(""+arg[0]+"-"+arg[1], arg[0], arg[1]);
    }
    
    load(array(test3));
    for(long[] arg : new long[][] {
      { 100, 200 },
      { 90, 100 },
      { 95, 150 },
      { 180, 250 },
      { 120, 150 }
    }) {
      resetExpected();
      expect(test3, arg[0], arg[1]);
//      System.out.println("Preparing to read: "+arg[0]+"-"+arg[1]);
//      dump();
      testRead(""+arg[0]+"-"+arg[1], arg[0], arg[1]);
    }
  }
    
}