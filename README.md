lighDB - light and easy to use embed database written on C99. It has minimal memory footprint and very fast. It designed to work on MCU, but can work everywhere.

# Alternatives
* [KissDB](https://github.com/adamierymenko/kissdb) - awesome DB, but only key-value, caches hashtable, uses dynamic memory allocation. Only key-value.
* [PureDB](https://github.com/jedisct1/PureDB) - good DB too, but DB creation, write & read is separated from each other. Uses dynamic memory allocation. Only key-value.

# Pros
* C99
* Does not require dynamic allocation - only one small static buffer
* IO functions wrapped up. Doesn't requires STD read, write and etc - so you can use any other FS lib, like Elm Chan FATFS or etc.
* Index or ID or hash addressinga
* You can write and read at any time
* Mutexes

# Cons
* No search through data only IDs or hashes


# LICENSE

BSD 2-Clause License

Copyright (c) 2017, Alexander Lutsai
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
