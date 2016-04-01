Read Me:

What the sanitizers do:
The sanitizers take an unsafe string (tagged by the htmlparser), removes and/or replaces unsafe characters and returns a safe alternative to the string.

How to add sanitizers:
1. Write a new sanitizer function to sanitizers.c
2. Each sanitizer function takes a string and its length.
3. Each sanitizer function returns a new sanitized string and its len as an arguement.