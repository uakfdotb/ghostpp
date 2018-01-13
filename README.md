GHost++
=======

GHost++ is a Warcraft III game hosting bot. As the original project on Google Code is no longer available, https://github.com/uakfdotb/ghostpp contains the most "official" version of GHost++.

* Github: https://github.com/uakfdotb/ghostpp
* Website: https://www.ghostpp.com/

GHost++ is no longer actively maintained. Nevertheless, GHost++ remains widely used for hosting Warcraft III games, and there are no major known bugs.

If you are looking for a simple bot to host games from a server, consider [Aura](https://github.com/Josko/aura-bot/), an actively maintained fork of GHost++ with a greatly modernized core but with many features stripped out.

Compilation
-----------

GHost++ depends on libboost, libgmp, zlib, libbz2, and libmysqlclient. These steps should suffice to compile GHost++ on Ubuntu 16.04:

	sudo apt-get install -y git libboost-all-dev build-essential libgmp-dev zlib1g-dev libbz2-dev libmysql++-dev
	git clone https://github.com/uakfdotb/ghostpp
	cd ghostpp
	cd bncsutil/src/bncsutil/
	make && sudo make install
	cd ../../../StormLib/stormlib/
	make && sudo make install
	cd ../../ghost
	make

See MANUAL or [the ghostpp.com wiki](https://www.ghostpp.com/wiki/index.php?title=Main_Page) for more in-depth but possibly outdated guides on other platforms.

Configuration
-------------

Generally, it is recommended to copy `default.cfg` to `ghost.cfg`, and update options there. GHost++ will read `default.cfg` first, and then overwrite the configuration with any options that appear in `ghost.cfg`.

Once configured, start GHost++:

	./ghost++

You can pass a command-line argument to use a different secondary configuration filename, instead of `ghost.cfg`:

	./ghost++ /opt/myconfig.cfg

Usage
-----

See MANUAL or [the ghostpp.com wiki](https://www.ghostpp.com/wiki/index.php?title=Main_Page) for instructions on using GHost++.
