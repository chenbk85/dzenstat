ayekat/dzenstat README
======================


*dzenstat* is a simple monitor that prints system information out to the
console. It is meant to be used in combination with
<a href="http://github.com/robm/dzen/">dzen2</a>.

Please note that this program is still in early development, and thus it is not
meant to be used. And even as it moves on, it's just meant to work for my needs.

But of course you may use the source code to build your own little tools.


have a screenshot!
------------------

Just to give you an impression what it should look like - and what it currently
looks like.

![screenshot](http://ayekat.ch/img/host/screen_dzenstat.png)

above: **conky**

below: **dzenstat**


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

	dzenstat | dzen2 -ta r -w 1000

This will put a dzenstat bar on the upper right corner of your screen 1000
pixels wide.

Run <code>dzen2 --help</code> or visit the dzen2 page for information about its
options.

Please note that currently there is no way to install dzenstat on the system, so
dzenstat will assume that you invoke it from the very directory the *icons*
folder lies in. This is crucial for correctly displaying the icons.

A <code>PKGBUILD</code> is in on the way!


what's wrong with conky?
------------------------

Nothing is wrong with conky. It's a general-purpose program that is invaluable
for displaying various kinds of system information, in a easy way.

Yet if my <code>.conkyrc</code> contains lines like

	${execi 5 sensors | grep temp1 | awk '{print $2}' | tail -n 1}

on a 1GHz netbook, I can't simply ignore the application footprint. That's why I
came up with the idea to write my own 'script' in C; it's a fun way to learn
something about C and the operating system.

