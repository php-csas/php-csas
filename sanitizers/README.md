## Overview

The sanitizers take an unsafe string (tagged by the htmlparser), removes and/or replaces unsafe characters and returns a safe alternative to the string.

How to add new sanitizers:
- Write a new sanitizer function, add it to sanitizers/sanitizers.c
- Each sanitizer function takes a string and its length.
- Each sanitizer function returns a new sanitized string and the new length as an arguement.
