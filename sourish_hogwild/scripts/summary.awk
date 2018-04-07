#!/bin/gawk -f
BEGIN { max = 0 }{ s+= $1; if($1 > max) max = $1 } END { print "rows", NR, "avg", s/NR, "max", max}