user=`whoami`
echo "*** Aborting all of "$user"'s jobs ***"

hold=`qselect -s Q -u $user`
if [ "$hold" != "" ]
then
  echo "Puting jobs on hold:"
  echo $hold
  qhold $hold
else
  echo "No jobs to put on hold..."
fi

all="`qselect -u $user`"
if [ "$all" != "" ]
then
  echo "Deleting jobs:"
  echo $all
  qdel $all
else
  echo "No jobs to delete..."
fi

echo Done!
