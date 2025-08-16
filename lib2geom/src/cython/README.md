# Installing:

In addition to 2geom dependencies, cython bindings need `cython >= 0.16`.

You can turn them on using cmake. Please note that you need to enable
shared library option as well.

Building on Windows is not tested yet, should be done shortly. Extrapolating
from other projects using cython, this should not be major problem.

# Usage:

Bindings are almost 1-1 to 2geom, so using them is pretty straightforward.
Classes and methods are documented shortly. It's generally good idea to
look at 2geom docs and, if problems persist, at their source.

To look at simple use cases, I suggest looking at tests and utils.py, located
in cython-bindings directory.

# Hacking:

cython is pretty straightforward to pick up, but its docs usually cover only
the simplest example. Looking at source of other project can be helpful
(cython bindings for SFML 2 are good example).

Don't hesitate to contact me or 2geom mailinglist with any requests concerning
design of bindings and bug reports.

Jan Pulmann - jan.pulmann@gmail.com
