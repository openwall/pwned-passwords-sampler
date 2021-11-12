Generate representative samples from Pwned Passwords (HIBP)
===========================================================

This program generates representative samples from Pwned Passwords (HIBP),
taking the count fields into account.

To use it, you need a file such as pwned-passwords-ntlm-ordered-by-hash-v7.txt
from https://haveibeenpwned.com/Passwords

Compile and invoke the program on Linux as follows:

```shell
$ gcc pwned-passwords-sampler.c -o pwned-passwords-sampler -O2 -s -Wall
$ ./pwned-passwords-sampler < pwned-passwords-ntlm-ordered-by-hash-v7.txt > pp-sample
Total 3650716681
$ wc -l pp-sample
1000000 pp-sample
```

With everything already optimally cached in RAM, this takes under 1 minute.

The input file is expected to use CRLF linefeeds exactly as provided by HIBP,
whereas the output has LF-only linefeeds.

You need to be on a 64-bit system with at least 32 GB RAM, preferably 64+ GB.
Usage on non-Linux might require minor changes to the code.
