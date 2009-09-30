#!/bin/bash
../../../build/bin/yaca-flash a.hex
echo booting app...
../../../build/bin/yaca-tr 1 0 0E
