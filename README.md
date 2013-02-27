ayekat/dzenstat README
======================


*dzenstat* is a simple monitor that prints system information out to the console.
It is meant to be used in combination with
<a href="http://github.com/robm/dzen/">dzen2</a>.

So to use dzenstat, you may run this command for example:

	dzenstat | dzen2 -ta r

See the dzen2 page for information about its options.


have a screenshot!
------------------

![screenshot](http://ayekat.ch/img/host/screen_dzenstat.png)


configure & build
-----------------

In the spirit of <a href="http://dwm.suckless.org/">dwm</a>, configuration is
simply done by editing the source code; the header file <code>config.h</code>
contains variables.

To build, simply compile with

	make

This should create a binary <code>dzenstat</code> in the same directory.


usability
---------

As this program is still in early development, it is not yet meant to be used.
And even as it moves on, it's just meant to work for my needs. But you may of
course use the source code to build your own little tools.

And anyway: even I don't use it yet.


what's wrong with conky?
------------------------

Nothing is wrong with conky. It's a general-purpose program that is invaluable
for displaying various kinds of system information, in a easy way.

Yet if I have lines like these

	${execi 5 sensors | grep temp1 | awk '{print $2}' | tail -n 1}

in <code>.conkyrc</code> on my 1GHz netbook, the application footprint still
starts to matter. That's why I thought I could write my own 'script' in C; it's
a funny way to learn a bit about the operating system.

