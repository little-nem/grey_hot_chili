# Building

To build the program, ensure that openmp is available on the building platform.

Just run
``` make ```

# Using it

One can run the program using

``` ./temp_name input.png ```

Help for more advanced options are available using

``` ./temp_name -h ```


# Remarks

We include the following external libraries:
    - http://optionparser.sourceforge.net/index.html for option parsing
    - https://github.com/nothings/stb for image loading and writing
    - https://github.com/vioshyvo/mrpt for efficient Nearest-Neighboor implementation
    - https://eigen.tuxfamily.org for efficient linear algebra

