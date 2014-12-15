package jitd;

import java.util.*;

public class TransitionMode extends Mode {
  int stepsTotal;
  int stepsTaken = 0;
  Random rand = new Random();
  Mode source, target;
  
  public TransitionMode(Mode source, Mode target, int steps)
  {
    this.stepsTotal = steps;
    this.source = source;
    this.target = target;
  }
  
  public Mode pick()
  {
    stepsTaken++;
    if(rand.nextInt(stepsTotal) < stepsTaken){
      return target;
    } else {
      return source;
    }
  }

  public KeyValueIterator scan(Driver driver, long low, long high)
  {
    return pick().scan(driver, low, high);
  }
  public void insert(Driver driver, Cog values)
  {
    pick().insert(driver, values);
  }
  public void idle(Driver driver)
  {
    pick().idle(driver);
  }
}