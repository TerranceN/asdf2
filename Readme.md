ASDF2
=====

Fast history searching tool.

Rewrite of [this script](https://github.com/TerranceN/asdf) using ncurses for blazing fast speed, hence the 2.

Running
-------

Add the cloned folder to your path and run `asdf`

- Up/Down -> select different commands

- Enter -> runs the selected command

- <C-Space> -> edit the selected command before running it

Building
--------

### Requirements:

- ncurses headers (you need to do something like `sudo apt-get install ncurses-dev` or `brew install ncurses`

- make

You should be able to just run `make`, and it should build fine assuming you have ncurses installed.
