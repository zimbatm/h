# `h` - faster shell navigation of projects
 
[![Built with Nix](https://builtwithnix.org/badge.svg)](https://builtwithnix.org)
[![Build Status](https://travis-ci.org/zimbatm/h.svg?branch=master)](https://travis-ci.org/zimbatm/h)

`h` is a small shell utility that I use every day to jump between projects quickly. It is complimentary to the amazing j
([autojump](https://github.com/joelthelion/autojump)) project and both help me
improve my workflow.

`h` is designed to work with a secific filesystem structure where all code is
checked-out in `~/code/<domain>/<path>`. Eg: `~/code/github.com/zimbatm/h` for
this project. This allows to not have to think about project locality.

The goal is that `h h` would find and cd into `~/code/github.com/zimbatm/h` if
it exists. When using the `h zimbatm/h` form it would look for the specific
folder or clone the repo from github. In both cases you will end-up changing
directory in the repo that you want to access. This allows to quickly access
existing and new project.

If projects don't live on github then their full git url can be provided to
clone into `~/code/<domain>/<path>`.

## Usage

`h <name>` searches for a project called `<name>` where `<name>` matches
`^\w\.\-$`. The search is done up to 3 levels deep and the longest match is
returned. If a result is found the path is printed on stdout. The
current directory is printed on stdout.

`h <user>/<repo>` looks for a `~/code/github.com/<user>/<repo>` folder or
clones it from github. The path is output on stdout if the repo exists or has
been cloned successfully. The current directory is printed on stdout
otherwise.

`h <url>` looks for a `~/code/<domain>/<path>` folder or clones it with git.
The path is output on stdout if the repo exists or has been cloned
successfully. The current directory is printed on stdout otherwise.

## Installation

Copy the `h` ruby script to somewhere in the PATH.

In your `~/.zshrc | ~/.bashrc | ..` add the following line:

```bash
eval "$(h --setup ~/code)"
```

This installs something really similar to `alias h='cd "$(h ~/code "$@")"`
where ~/code is the root of all your repositories. Once the shell is reloaded
you can use the above commands.

## See also

* [autojump](https://github.com/joelthelion/autojump)
* [hub](https://hub.github.com/)
* [direnv](http://direnv.net/)

## License

MIT - (c) 2015 zimbatm and contributors


