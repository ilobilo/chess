# Bf2os
**VERY** badly coded cli chess

## Building and Running
* ``meson setup builddir``
* ``ninja -C builddir``
* ``./builddir/chess -h/-v``
* ``./builddir/chess``

## TODO
* Castles
* Get in front of the king or kill the *checker piece* when being checked
* King should not be able to take a piece if it's protected by other piece