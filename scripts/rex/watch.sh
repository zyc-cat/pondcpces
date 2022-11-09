delay=1
if [ $# -gt 0 ]; then
  delay=$1
fi
while [ true ]
do
  clear
#  qstat -q
  qstat -Q mpi_medium
  sleep $delay
done
