# Overview

Below is a test case that needs to be done in a branch of the php-csas repo to make sure we can output data after sanitization to the browser again, completing the process.

# Do This

- Create a new branch in the php-csas repo named `sanitization-to-browser`
- In the extension do the following:
	- Modify the echo_handler function to make any string echoed or printed be "hello world"
- Create a PHP file following the outline below:
```declare a string that is not the same length as the string "hello world" and is not "hello world"
echo length of string declared
echo string declared
echo length of string declared
```
- Run this PHP file in our apache webserver and confirm the following:
	- length of the string needs to be unchanged from the original declaration of a string other than "hello world"
		- AKA the first string length echoed should be the same as the second string length echoed.
	- the string output by the echo should still be "hello world"

- If the above two things are not confirmed by what was implemented in the extension as specified, then make the modification of the extension that changes every echoed and printed value "hello world" conform to the above two notes.

# Value of this

- By making sure we can do what was specified to add in this branch of the extension and then by confirming that the results are as stated, we will not have issues when outputting the sanitized values from CSAS back to the browser.
- The underlying issue is that we do not want to mess up the properties of the inputted values except for sanitizing them. We use the string length metric to demonstrate that in the case a programmer wants to use user input after it was sanitized, that the properties of the user input before sanitization are *the same* as after sanitization.