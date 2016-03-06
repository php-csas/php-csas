# PHP-CSAS

Context-Sensitive Auto-Sanitization for PHP

## Overview
- File Structure Overview:
	- `config.m4` and `config.w32`: C config file for the CSAS extension.
	- `csas.c`: C Source file for the extension.
	- `php_csas.h`: C Header file for the extension.

## To install
- `mv $PHP_HOME/php.ini-Development $PHP_HOME/php-install-directory/lib/php.ini`
- Add the lines `extension=taint.so` and `taint.enable = 1` to the installed PHP's php.ini file. This is in `PHP_HOME/php-install-directory/lib/php.ini` for the purposes of this project (in reference to the dev environment).
- Run `build_extension.sh`.

## To install (Old instructions)
- Make sure that the debian package `autoconf` is installed. Run `sudo apt-get install autoconf` to check. Install it if its not already installed.
- Make sure that `$PHPDIR/php-install-directory/bin` is in your path. Check with `echo $PATH`.
- Make sure that `php.ini` exists at `$PHPDIR/php-install-directory/lib`. If it does not, copy `php.ini-development` from `$PHPDIR` to that location.
- Add `extension=csas.so` to the end of the `php.ini` file.
- In the PHP-CSAS directory, run the following sequence of commands to install the extension:
	- `phpize`: Creates all of the necessary configure and make files to the extension repository. Don't worry, all of these created files are in the `.gitignore`.
	- `./configure`: Configure the extension.
	- `make`: Build the extension.
	- `sudo make install`: Install the extension.
		- You should see something along the lines of `Installing shared extensions: $PHPDIR/php-install-directory/lib/php/extensions/debug-zts-20090626/`
- To test that this works, run `php -r 'csas_main();'`.
	- If it prints out `Hello, CSAS!` then it installed into PHP correctly!
# test-sites
Place these files in your web root to test CSAS.
