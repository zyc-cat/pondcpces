tmp_suffix=
tmp_dir=temp$tmp_suffix

prob_dir=~/pond/rddlsim/files/final_comp/ppddl/
#pushd $prob_dir
#suite=`ls *.po-ppddl`
#popd

suite=(
crossing_traffic_inst_pomdp__1.po-ppddl
elevators_inst_pomdp__1.po-ppddl
game_of_life_inst_pomdp__1.po-ppddl
navigation_inst_pomdp__1.po-ppddl
recon_inst_pomdp__1.po-ppddl
skill_teaching_inst_pomdp__1.po-ppddl
sysadmin_inst_pomdp__1.po-ppddl
traffic_inst_pomdp__1.po-ppddl
crossing_traffic_inst_pomdp__2.po-ppddl
elevators_inst_pomdp__2.po-ppddl
game_of_life_inst_pomdp__2.po-ppddl
navigation_inst_pomdp__2.po-ppddl
recon_inst_pomdp__2.po-ppddl
skill_teaching_inst_pomdp__2.po-ppddl
sysadmin_inst_pomdp__2.po-ppddl
traffic_inst_pomdp__2.po-ppddl
#crossing_traffic_inst_pomdp__3.po-ppddl
#elevators_inst_pomdp__3.po-ppddl
#game_of_life_inst_pomdp__3.po-ppddl
#navigation_inst_pomdp__3.po-ppddl
#recon_inst_pomdp__3.po-ppddl
#skill_teaching_inst_pomdp__3.po-ppddl
#sysadmin_inst_pomdp__3.po-ppddl
#traffic_inst_pomdp__3.po-ppddl
#crossing_traffic_inst_pomdp__4.po-ppddl
#elevators_inst_pomdp__4.po-ppddl
#game_of_life_inst_pomdp__4.po-ppddl
#navigation_inst_pomdp__4.po-ppddl
#recon_inst_pomdp__4.po-ppddl
#skill_teaching_inst_pomdp__4.po-ppddl
#sysadmin_inst_pomdp__4.po-ppddl
#traffic_inst_pomdp__4.po-ppddl
#crossing_traffic_inst_pomdp__5.po-ppddl
#elevators_inst_pomdp__5.po-ppddl
#game_of_life_inst_pomdp__5.po-ppddl
#navigation_inst_pomdp__5.po-ppddl
#recon_inst_pomdp__5.po-ppddl
#skill_teaching_inst_pomdp__5.po-ppddl
#sysadmin_inst_pomdp__5.po-ppddl
#traffic_inst_pomdp__5.po-ppddl
#crossing_traffic_inst_pomdp__6.po-ppddl
#elevators_inst_pomdp__6.po-ppddl
#game_of_life_inst_pomdp__6.po-ppddl
#navigation_inst_pomdp__6.po-ppddl
#recon_inst_pomdp__6.po-ppddl
#skill_teaching_inst_pomdp__6.po-ppddl
#sysadmin_inst_pomdp__6.po-ppddl
#traffic_inst_pomdp__6.po-ppddl
#crossing_traffic_inst_pomdp__7.po-ppddl
#elevators_inst_pomdp__7.po-ppddl
#game_of_life_inst_pomdp__7.po-ppddl
#navigation_inst_pomdp__7.po-ppddl
#recon_inst_pomdp__7.po-ppddl
#skill_teaching_inst_pomdp__7.po-ppddl
#sysadmin_inst_pomdp__7.po-ppddl
#traffic_inst_pomdp__7.po-ppddl
#crossing_traffic_inst_pomdp__8.po-ppddl
#elevators_inst_pomdp__8.po-ppddl
#game_of_life_inst_pomdp__8.po-ppddl
#navigation_inst_pomdp__8.po-ppddl
#recon_inst_pomdp__8.po-ppddl
#skill_teaching_inst_pomdp__8.po-ppddl
#sysadmin_inst_pomdp__8.po-ppddl
#traffic_inst_pomdp__8.po-ppddl
#crossing_traffic_inst_pomdp__9.po-ppddl
#elevators_inst_pomdp__9.po-ppddl
#game_of_life_inst_pomdp__9.po-ppddl
#navigation_inst_pomdp__9.po-ppddl
#recon_inst_pomdp__9.po-ppddl
#skill_teaching_inst_pomdp__9.po-ppddl
#sysadmin_inst_pomdp__9.po-ppddl
#traffic_inst_pomdp__9.po-ppddl
#crossing_traffic_inst_pomdp__10.po-ppddl
#elevators_inst_pomdp__10.po-ppddl
#game_of_life_inst_pomdp__10.po-ppddl
#navigation_inst_pomdp__10.po-ppddl
#recon_inst_pomdp__10.po-ppddl
#skill_teaching_inst_pomdp__10.po-ppddl
#sysadmin_inst_pomdp__10.po-ppddl
#traffic_inst_pomdp__10.po-ppddl
)


echo "*** Queuing up tests ***"

rm -r -f $tmp_dir
mkdir -p $tmp_dir

server=0
for problem in ${suite[@]}
do
  port_server=$server
  if [ $port_server -lt 10 ]
  then
    port_server=0$port_server
  fi
  port_server=23$port_server

  port_client=$server
  if [ $port_client -lt 10 ]
  then
    port_client=0$port_client
  fi
  port_client=24$port_client

  prb_name=${problem/\//_}
  prb_name=${prb_name/./_}

  tmp_filename=$tmp_dir"/"$prb_name".sh"
  echo "ulimit -m 7340032" > $tmp_filename
  echo "ulimit -v 7340032" >> $tmp_filename
  echo "cd ~/pond/rddlsim" >> $tmp_filename
  echo "./run rddl.competition.Server files/final_comp/rddl/ "$port_server" &" >> $tmp_filename
  echo "sleep 5s" >> $tmp_filename
  echo "cd ~/pond/src" >> $tmp_filename
  echo "rddlclient/rddlclient -p "$prob_dir$problem" -s localhost:"$port_server" -c localhost:"$port_client" -rbpf 32 -pmg 32 -depth 10 -samples 3 -mp 32 -o ~/pond/results/h_32_32_10_3_32/" >> $tmp_filename
  qsub -mn -q mpi_medium -lnodes=1:ppn=8,mem=15gb,walltime=3:00:00 $tmp_filename

  let server++
done

echo Done!
