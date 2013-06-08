ayekat/dzenstat README
======================


*dzenstat* is a simple monitor that prints system information out to the
console. It is meant to be used in combination with
<a href="http://github.com/robm/dzen/">dzen2</a>.

Please note that this program is still in early development, so it's very likely
that things are buggy. However it should not break anything on the system, so
feel free to test!


screenshot
----------

Now with MPD support!

![screenshot](http://ayekat.ch/img/host/github.com/screen_dzenstat.png)


configure
---------

dzenstat is organised in *modules* - you can think of them as "widgets".

<code>src/modules.h</code> contains an array that, for each module, holds the
corresponding function to be called to initialise the module; dzenstat will
arrange them in the same order (except date and time; they are currently
hardcoded in the <code>display()</code> function, but will hopefully get
exported to a module, too).

To configure the modules, <code>src/config</code> holds a bunch of header files
that contain variables to customise the behaviour of the modules.

If you want to fine-tune them further, you should also take a look in the
modules' <code>.c</code> files themselves (I hope the code is readable). Once
you get the general idea of how it works, you might also consider adding modules
on your own!


build
-----

To build, compile with

	make

This should create a binary <code>dzenstat</code> in the same directory.


run
---

As mentioned above, dzenstat is meant to be used in combination with dzen2:

	./dzenstat | dzen2 -ta r

This will put a dzenstat bar on the upper right corner of your screen.

Run <code>dzen2 --help</code> or visit the
<a href="http://github.com/robm/dzen/">dzen2</a> page for information about its
options.


some notes
----------

Currently there are not scripts to install dzenstat on the system, therefore it
must be invoked as described above. But of course you can change the
<code>path\_icons</code> variable in the global config file to something like
<code>/usr/share/dzenstat/icons</code>, put the icons there and install dzenstat
to <code>/usr/bin</code>.

However, as a sane default (= without configuring and installing), dzenstat will
assume that you invoke it from the very directory the *icons* folder lies in.

