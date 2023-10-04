# Windows Caveats

1. ReLarn does not really know about Unicode, so non-ASCII characters
   in paths and environment variables may cause problems.

2. ReLarn expects the environment variables `HOMEDRIVE` and `HOMEPATH`
   to point to your profile directory and your "My Documents" folder
   to be named "Documents".  (You can also make it use a different
   location by changing these.  This may help if (e.g.) you have Unicode
   characters in profile path.)

