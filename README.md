![SST](http://sst-simulator.org/img/sst-logo-small.png)
![LLVM](http://llvm.org/img/DragonSmall.png)

# LLVM DAG Estimator
Open-source supercomputing, possibly using "Silicon Compilers" are the way of the future as Dennard scaling has stopped and Moore's Law pushes its limits. Pushing performance means moving away from general purpose CPUs to custom accelerators designed to perform a specific purpose as quickly as possible with the minimum possible power consumption.

For certain codes, with hundreds of numerically intensive functions, a new question will need to be efficiently answered: is my function a good candidate for custom acceleration or should I leave it to a general purpose processor? Laid out on an FPGA or custom accelerator, what area - how much width (how many registers/logic blocks operating in parallel) and how much depth (how many steps in the pipeline) do I need to implement the algorithm?

## Building and Running
The code uses a standard CMake build system. The only prerequisite is LLVM >= 6. The build systems uses the LLVM CMake package to configure. In order to use, configure with:

````
cmake ${path_to_source} -DLLVM_dir=${path_to_lib_cmake_llvm}
````
It is recommended that a build directory is first created and then cmake called from within that directory. Following the build, an executable named "estimate" will be available in the bin directory. To print the available options for the estimate tool use the following:

````
./estimate -h
````
Note: available optimization levels are O0, O1, O2, O3, Os, and Oz, which are correspondingly called with 0, 1, 2, 3, s, and z.

## Directly Analyze C/C++ Source Code
This feature is not yet supported with the estimate tool.

## Generating IR Files
To generate an IR file from C/C++ source code to use with the IR option use the following command:

````
clang -emit-llvm -S -c ${path_to_source_code}
````
Use the estimate tool on the generated IR code with:

````
./estimate --opt ${opt_level} --ir ${path_to_IR_code}
````

## Generating and Viewing DOT Files
### Generating the DOT Files
To generate the DOT files for each llvm function analyzed refer to the following examples:

Example 1 - Generate the DOT files for the built-in tests:
```
./estimate --builtin ${test_number} --gendot 1
```

Example 2 - Generate the DOT files for a set of generated IR code:
```
./estimate --ir ${path_to_IR_code} --gendot 1
```

### Viewing the DOT Files
After generating the DOT files, follow these steps to view an image of the DAG:

```
cd ../../dotFiles
../scripts/dot_to_png
open .
```

## Built-In Tests
Each built-in test is summarized below.

### Test 1
A simple set of assignments, placed in a for loop, to test the width and depth results of the estimate tool. The width and depth for the variables within this function will be observed in the results only if the optimization level is set to '0'. 

### Test 2
Test 2 is a function that makes a call to the IR function generated in Test 1. The output shows the result of a simplified outer function, but the inner call to the Test 1 function remains unsimplified.

### Test 3
Test 3 contains an if-else statement nested within a for loop. Currently, the ability to analyze if-statements is not yet supported with the estimate tool.

### Running the Built-In Tests
Run the built in tests with the following command:

````
./estimate --opt ${opt_level} --builtin ${test_number} --iter ${iterations}
````

## Analyzing a Specific Function
To analyze a specific function within a module of generated IR code use the fxn flag. Such a call would have the following form:

````
./estimate --opt ${opt_level} --ir ${path_to_IR_code} --fxn ${name_of_IR_function}
```` 


[![License](https://img.shields.io/badge/License-BSD%203--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)

Under the terms of Contract DE-NA0003525 with NTESS,
the U.S. Government retains certain rights in this software.

