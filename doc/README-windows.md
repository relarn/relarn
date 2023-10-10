# Windows Caveats

**Note:** Since I virtually never use Windows anymore and don't have a
development environment set up for it, Windows support will be weak at
best.  If someone wants to take it over, please contact me.

1. ReLarn does not really know about Unicode, so non-ASCII characters
   in paths and environment variables may cause problems.

2. ReLarn expects the environment variables `HOMEDRIVE` and `HOMEPATH`
   to point to your profile directory and your "My Documents" folder
   to be named "Documents".  (You can also make it use a different
   location by changing these.  This may help if (e.g.) you have Unicode
   characters in profile path.)

