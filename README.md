ayekat/dwmzen README
====================


*dwmzen* is a simple monitor that prints system information out to the console.
It is meant to be used in combination with
<a href="http://github.com/robm/dzen/">dzen2</a>.

So to use dwmzen, you may run this command for example:

	dwmstat | dzen2 -ta r

See the dzen2 page for information about its options.


configure & build
-----------------

In the spirit of <a href="http://dwm.suckless.org/">dwm</a>, configuration is
simply done by editing the source code; the header file <code>config.h</code>
contains variables.

As this program is still in early development, it is not yet meant to be used.

Or rather: I don't use it yet.


what's wrong with conky?
------------------------

Nothing is wrong with conky. It's a general-purpose program that is invaluable
for displaying various kinds of system information, in a easy way.

Yet if I have lines like these

	${execi 5 sensors | grep temp1 | awk '{print $2}' | tail -n 1}

in <code>.conkyrc</code> on my 1GHz netbook, the application footprint still
starts to matter. That's why I thought I could write my own 'script' in C; it's
a funny way to learn a bit about the operating system.

