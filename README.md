ayekat/dzenstat README
======================


*dzenstat* is a simple monitor that prints system information out to the
console. It is meant to be used in combination with
<a href="http://github.com/robm/dzen/">dzen2</a>.

Please note that this program is still in early development, so it's very likely
that things are buggy.

However it should not break anything on the system, so feel free to test!


have a screenshot!
------------------

Just to give you an impression what it looks like:

![screenshot](http://ayekat.ch/img/host/screen_dzenstat4.png)



configure
---------

dzenstat is organised in *modules* -- you can think of them as "widgets".

<code>src/modules.h</code> contains an array that, for each module, holds the
corresponding function to be called to initialise the module; dzenstat will
arrange them in the same order.

In order to fine-tune the modules, <code>src/config</code> holds a bunch of
header files to configure each of them.

THE 'MODULES' WAY OF CONFIGURING DZENSTAT HAS ONLY RECENTLY BEEN ADDED -- EXPECT
BUGS!


build
-----

To build, compile with

	make

This should create a binary <code>dzenstat</code> in the same directory.


run
---

As mentioned above, dzenstat is meant to be used in combination with dzen2:

	dzenstat | dzen2 -ta r

This will put a dzenstat bar on the upper right corner of your screen.

Run <code>dzen2 --help</code> or visit the
<a href="http://github.com/robm/dzen/">dzen2</a> page for information about its
options.

dzenstat will assume that you invoke it from the very directory the *icons*
folder lies in. This is crucial for correctly displaying the icons.

