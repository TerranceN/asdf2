ASDF2
=====

Fast history searching tool.

Rewrite of [this script](https://github.com/TerranceN/asdf) using ncurses for blazing fast speed, hence the 2.

##### Using asdf to search for a command and running it:

![Example of running asdf](http://g.recordit.co/dovCQ6b1Gw.gif)

##### Using asdf to search for a command, and edit it before running:

![Example of editing a command](http://g.recordit.co/KDcDNiKbx7.gif)

Running
-------

Add the cloned folder to your path and run `asdf`.

Certain bash commands (like `cd some/path`) won't run correctly because this will be inside of a subshell, which then exits, losing your changes. In order to get around that you can source `asdf` instead of just running it. Simply add `alias asdf="source /path/to/asdf"` to your `.bashrc`.

Controls:

- `<Up>`/`<Down>` -> select different commands

- `<Enter>` -> runs the selected command

- `<C-Space>` -> edit the selected command before running it

TODO:
-----

- Add support for deleting commands from your `$HISTFILE`

- Add support for bash movement hotkeys (like `^W` deleting a word)

- Add regex support (assuming it doesn't slow things down too much)

- Add options for things like whether to add `asdf`-run commands to your `$HISTFILE` (currently it does by default)

Building
--------

### Requirements:

- ncurses development headers/libraries (you need to do something like `sudo apt-get install ncurses-dev` on Ubuntu, or `brew install ncurses` on Mac, or install the `libncurses-devel` package on Windows/Cygwin)

- make

You should be able to just run `make`, and it should build fine assuming you have ncurses installed.
