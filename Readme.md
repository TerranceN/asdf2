ASDF2
=====

Fast history searching tool for bash.

Rewrite of [this script](https://github.com/TerranceN/asdf) using ncurses for blazing fast speed, hence the 2.

#### Using asdf to search for a command and running it:

![Example of running asdf](http://i.imgur.com/gXyuije.gif)

#### Using asdf to search for a command, and edit it before running:

![Example of editing a command](http://i.imgur.com/8NgLbI5.gif)

#### Using asdf to delete a command from the history:

![Example of deleting a command](http://i.imgur.com/nWuVLVq.gif)

Building
--------

### Requirements:

- ncurses development headers/libraries (you need to do something like `sudo apt-get install ncurses-dev` on Ubuntu, or `brew install ncurses` on Mac, or install the `libncurses-devel` package on Windows/Cygwin)

- bash 4 ([Mac install instructions](http://stackoverflow.com/a/16416694))

- make

You should be able to just run `make`, and it should build fine assuming you have ncurses installed.

Running
-------

After you've built it simply run `./asdf`. You can add it to your `PATH` to be able to use it from anywhere. Certain bash commands (like `cd some/path`) won't run correctly because this will be inside of a subshell, which then exits, losing your changes. In order to get around that you can source `asdf` instead of just running it. Simply add `alias asdf="source /path/to/asdf"` to your `.bashrc`.

Controls:

- `<Up>`/`<Down>` -> select different commands

- `<Enter>` -> runs the selected command

- `<C-Space>` -> edit the selected command before running it

- `<C-Del>` (or `<C-\>` on Mac) -> remove the selected command (and any copies) from your HISTFILE

#### Extras

Not required, but adding the following to your `.bashrc` can make interacting with your history a little bit nicer:

```bash
# make history virtually unlimited
export HISTSIZE=1000000
export HISTFILESIZE=$HISTSIZE

# make it so that asdf doesn't show up in the history
export HISTIGNORE="$HISTIGNORE:asdf*"

# append commands to the history as they're run (instead of after the shell exits)
shopt -s histappend
PROMPT_COMMAND="history -a;$PROMPT_COMMAND"
```

TODO:
-----

- Clean up the code. It's kind of a mess right now. Manageable, but a mess.

- Add support for other shells (~~zsh~~ and fish at least)

- Add support for bash movement hotkeys (like `^W` deleting a word)

- Add regex support (assuming it doesn't slow things down too much)
