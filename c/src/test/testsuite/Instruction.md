Format is as follows


--insert random <size of array with random numbers from range 0 to 1 million>
--insert <value to place into cog>
--workload <workload type> <rebalance> <number of reads> <key range> <time_pattern> <sort or not sort>
--heavyhit <key shift> <lower bound> <upper bound> <hot data fraction> <hot access fraction>
--heavyshift <amount you want heavy hitter value to be shifted by>
--crack
--crackmerge

**for workload generator type, 0 is random, 1 is zipfian, 2 is heavyhitter
 * 0 for time total, 1 for time each read
 * don't need to create a generator for zipfian or random, workload can do it if specified
 i.e.
 heavyhit 0 0 1000000 0.1 0.8
 heavyshift 500000
 run
