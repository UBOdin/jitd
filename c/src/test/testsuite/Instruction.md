Format is as follows

--workload <workload type> <rebalance> <# of reads> <key range>
--heavyhit <key shift> <lower bound> <upper bound> <hot data fraction> <hot access fraction>

**for workload generator, 0 is random, 1 is zipfian, 2 is heavyhitter
 * don't need to create a generator for zipfian, workload can do it if specified
