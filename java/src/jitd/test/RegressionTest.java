package jitd.test;

import java.util.*;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import org.junit.*;
import org.junit.runner.*;
import org.junit.runners.*;
import org.junit.runners.Parameterized.Parameters;

import jitd.*;

public class RegressionTest extends CogTest {

  public Driver merge(Cog root)
  {
    return new Driver(new PushdownMergeMode(), root);
  }
  
  public Driver crack(Cog root)
  {
    return new Driver(new CrackerMode(), root);
  }
  
  @Test
  public void simBadger()
  {
    srand(42); 
    
    Cog root = array(randArray(20000, 1000000));
    Driver d = crack(root);
    Mode merge = new PushdownMergeMode();
    Mode crack = d.mode;
    
    d.dump();
    d.scan(100000, 180000);
    d.dump();
    d.root = concat(
      d.root, 
      array(randArray(20000, 1000000))
    );
    d.dump();
    d.scan(40000, 80000);
    d.dump();
    d.mode = merge;
    d.scan(110000, 115000);
    d.dump();
    d.scan(115000, 2000000);
    d.dump();
  }
  @Ignore
  @Test 
  public void adaptiveMergeShouldWipeOutEmptyPartitions()
  {
    Cog root = 
      bnode(8656, 
        concat(
          bnode(9712,
            sorted(new long[] { 8656, 9500 }),
            sorted(new long[] { 9800, 9900 })
          ),
          bnode(8656, 
            sorted(new long[] { 8656, 8728 }),
            empty()
          )
        ),
        sorted(new long[] { 1000, 2000, 3000, 4000, 5000 })
      );
    Driver d = merge(root);
    
    d.dump();
    d.scan(8656, 8700);
    d.dump();
    
    
  }
}