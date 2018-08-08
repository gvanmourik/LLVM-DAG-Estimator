![SST](http://sst-simulator.org/img/sst-logo-small.png)
![LLVM](http://llvm.org/img/DragonSmall.png)

# LLVM DAG Estimator
Open-source supercomputing, possibly using "Silicon Compilers" are the way of the future as Dennard scaling has stopped and Moore's Law pushes its limits. Pushing performance means moving away from general purpose CPUs to custom accelerators designed to perform a specific purpose as quickly as possible with the minimum possible power consumption.

For certain codes, 
For large codes with hundreds of numerically intensive functions, a new question will need to be efficiently answered: is my function a good candidate for custom acceleration or should I leave it to a general purpose processor? Laid out on an FPGA or custom accelerator, what area - how much width (how many registers/logic blocks operating in parallel) and how much depth (how many steps in the pipeline) do I need to implement the algorithm?

[![License](https://img.shields.io/badge/License-BSD%203--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)

Under the terms of Contract DE-NA0003525 with NTESS,
the U.S. Government retains certain rights in this software.

