# swish

A swift shell written in C

# Installation

run `make`
execute with `./swish`
to clean, `make clean`

## Assumptions

1. Sudo permissions are required for pinfo.
2. There are size limits to various strings, arrays given in headers/def.h
3. Assuming that in spec 7, there exists at max only one filename and one directory name - so that even if they are in any order, we can parse it.
