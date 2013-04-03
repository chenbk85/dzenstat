ayekat/dzenstat README
======================


*dzenstat* is a simple monitor that prints system information out to the
console. It is meant to be used in combination with
<a href="http://github.com/robm/dzen/">dzen2</a>.

Please note that this program is still in early development, and meant to work
mainly for my own needs.

But of course you may use the source code to build your own little tools.


have a screenshot!
------------------

Just to give you an impression what it looks like:

![screenshot](http://ayekat.ch/img/host/screen_dzenstat.png)



configure & build
-----------------

In the spirit of <a href="http://dwm.suckless.org/">dwm</a>, configuration is
simply done by editing the source code; the header file <code>config.h</code>
contains variables.

To build, simply compile with

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

