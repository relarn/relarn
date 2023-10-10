
# ReLarn

ReLarn is an old-school Roguelike game based on ULarn.  The source
code was forked from Ularm 1.5-ish and extensively refactored to be
more maintainable and better suited to the modern world.

The code has been modularized with OS- and UI-dependencies abstracted
away, all warnings have been fixed, arrays replaced with structures
and more.  In addition, there have been a number of quality-of-life
improvements to the game and its interface as well as a few lore and
gameplay tweaks.

The goal of this project is to preserve the spirit of the original
game on modern computers.

See [Changes.md](Changes.md) for a more complete summary of changes.

* [Website](http://relarn.org)
* [Gitlab](https://gitlab.com/relarn/relarn)


## Getting Started

Pre-built binaries for the major platforms (and Windows) should now be
available from [the website](http://relarn.org) so the easiest route
is probably to just snag one of those and unzip it somewhere.

You should probably also at least skim [the manual](doc/relarn.pod).

If you want to build it yourself, directions are [here](doc/BUILD.md).


## Contributing

Pull requests are welcome.  In particular, I'm looking for bug fixes,
UI improvements and ports to new platforms.

Note that the goal of this project is to preserve the spirit of the
original game; as such, I'm not looking for new gameplay elements
(e.g. weapons, monsters, quests, etc., although stuff like balance
tweaks may be considered.)  If you're trying to create a different
game, you should fork this project and do it that way.


## Authors

See the file [AUTHORS.txt](AUTHORS.txt) for the list of known
contributors.  The Larn games have been hacked on for close to three
decades so it's impossible to track down everyone.

## License

This project is licensed under the GNU General Public License
version 2.  See [Copyright.txt](Copyright.txt) and
[LICENSE.txt](LICENSE.txt) for details.

