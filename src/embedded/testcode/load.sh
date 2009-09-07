#!/bin/bash
/home/david/Code/svn-yaca/yaca-x86/ycflash/trunk/build/main a.hex
echo booting app...
/home/david/Code/svn-yaca/yaca-x86/ycflash/branches/yctr/build/main 1 0 0E
