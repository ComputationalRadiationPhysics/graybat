#! /usr/bin/env fish

set PROCESSES $argv[1]
set N_SOCKETS 4
set N_CORES_PER_SOCKET 16
set FILENAME "rankfile_$PROCESSES"

rm $FILENAME

for i in (seq 0 (math $PROCESSES - 1)) 
  set SOCKET (math "$i % $N_SOCKETS")
  set CORE (math "$i % $N_CORES_PER_SOCKET")
  echo -n "rank $i=localhost" >> $FILENAME
  echo " slot=$SOCKET:$CORE"  >> $FILENAME

end

