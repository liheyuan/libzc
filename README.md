<a href="https://scan.coverity.com/projects/mferland-libzc">
  <img alt="Coverity Scan Build Status"
       src="https://scan.coverity.com/projects/7176/badge.svg"/>
</a>

<a href="https://travis-ci.org/mferland/libzc">
   <img alt="Build Status"
        src="https://travis-ci.org/mferland/libzc.svg?branch=master"/>
</a>

What is it?
===========
The libzc library is a simple zip cracking library. It also comes with
a command line tool called 'yazc' (Yet Another Zip Cracker).

How to install it?
==================
Just clone, configure, compile and install.

    git clone https://github.com/mferland/libzc.git
    cd libzc
    ./autogen.sh
    ./configure CFLAGS='-Ofast -march=native -mtune=native'
    make
    sudo make install

How to use it?
==============
There are currently 3 attack modes available:

Bruteforce
----------
This mode tries all possible passwords from the given character
set. It supports multi-threading.

Example:
Try all passwords in [a-z0-9] up to 8 characters with 4 threads:

    yazc bruteforce -a -n -l8 -t4 archive.zip

Dictionary
----------
This mode tries all passwords from the given dictionary file. If no
password file is given as argument it reads from stdin.

Examples:
Try all password from words.dict:

    cat words.dict | yazc dictionary archive.zip

Use John The Ripper to generate more passwords:

    john --wordlist=words.dict --rules --stdout | yazc dictionary archive.zip

Plaintext
---------
This mode uses a known vulnerability in the pkzip stream cipher to
find the internal representation of the encryption key. Once the
internal representation of the key has been found, we try to find the
actual (or an equivalent) password.

Example:
Try to find archive.zip password by using plain.bin using 8 threads:

    yazc plaintext -t8 plain.bin:100:650 archive.zip:112:662:64

TODO
----
- Support for GPU bruteforce cracking.
- Add basic mangling rules to dictionary attack.
- Review library api, should be much simpler.
- Optionally decrypt the archive (plaintext).
- Provide a way to benchmark libzc.
