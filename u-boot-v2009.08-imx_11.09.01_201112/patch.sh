#!/bin/bash
ls u-boot-v2009.08-imx_11.09.01_201112/*.patch | while read i; do
	echo "--- applying $i"
	patch -p1 --no-backup-if-mismatch < "$i"
	echo
done
