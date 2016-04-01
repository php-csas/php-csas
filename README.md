# PHP-CSAS
[![Build Status](https://travis-ci.org/php-csas/php-csas.svg?branch=master)](https://travis-ci.org/php-csas/php-csas)

- *One Goal*: Context-Sensitive Auto-Sanitization for PHP

## Overview
- We are building off of the work done by previous leaders in this field, including inspiration from a [paper from Google and UC Berkeley](http://webblaze.cs.berkeley.edu/papers/csas-ccs11.pdf). Languages like Java, Python, Ruby, and others have modules similar to PHP-CSAS for implementing auto-escaping for various template languages. Our goal is to bring the functionality of these languages into PHP. 



## File Structure Overview:
	- `config.m4` and `config.w32`: C config file for the CSAS extension.
	- `csas.c`: C Source file for the extension.
	- `php_csas.h`: C Header file for the extension.
	- `sanitizers/sanitizers.c`: Sanitizers used by PHP-CSAS on unsafe input.
	- `sanitizers/sanitizers/.h`: C Header file for sanitizers.c.
