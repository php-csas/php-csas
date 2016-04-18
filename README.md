# PHP-CSAS
[![Build Status](https://travis-ci.org/php-csas/php-csas.svg?branch=master)](https://travis-ci.org/php-csas/php-csas)

- *One Goal*: Context-Sensitive Auto-Sanitization for PHP

## Overview
- We are building off of the work done by previous leaders in this field, including inspiration from a [paper from Google and UC Berkeley](http://webblaze.cs.berkeley.edu/papers/csas-ccs11.pdf). Languages like Java ([Google Soy Templates](https://developers.google.com/closure/templates/docs/security#autoescaping)), C ([CTemplate](https://htmlpreview.github.io/?https://raw.githubusercontent.com/OlafvdSpek/ctemplate/master/doc/auto_escape.html)), and Python ([Django](https://docs.djangoproject.com/en/dev/ref/templates/builtins/#autoescape)) and others have modules similar to PHP-CSAS for implementing auto-escaping for various template languages. Our goal is to bring the functionality of these languages into PHP. 

## Documentation

The documentation is available in the [wiki](https://github.com/php-csas/php-csas/wiki).

## File Structure Overview:
- `config.m4` and `config.w32`: C config file for the CSAS extension.
- `csas.c`: C Source file for the extension.
- `php_csas.h`: C Header file for the extension.
- `sanitizers/sanitizers.c`: Sanitizers used by PHP-CSAS on unsafe input.
- `sanitizers/sanitizers.h`: C Header file for sanitizers.c.
- `htmlparser/`: Contains the HTML parsing code.
