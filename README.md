# h2o

This project solve simple h2o problem in C, it creates 2 processes for generating atoms (h, o). Those processes creating next processes, these new created processes are atoms.

When 2 atoms of Hydrogen and 1 atom of Oxygen are created, the bond can start, atom generation is paused until bonding is completed. All atoms then waits until all of them are turned into h2o.

This problem is solved with semaphores and barrier technique. I've used [The Little Book of Semaphores](http://www.greenteapress.com/semaphores/downey08semaphores.pdf) book from Allen B. Downey.

## Results

42/42 points