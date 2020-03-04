clear
Write-Output run_measures
Push-Location
cd measures


# config:
$prism_dir = "..\extern\prism-benchmarks\models\dtmcs"
$leader_sync = $prism_dir + "\leader_sync"
$list_of_leader_sync_files = "leader_sync3_2.pm","leader_sync3_3.pm","leader_sync3_4.pm","leader_sync3_5.pm","leader_sync3_6.pm","leader_sync3_8.pm","leader_sync4_2.pm","leader_sync4_3.pm","leader_sync4_4.pm","leader_sync4_5.pm","leader_sync4_6.pm","leader_sync4_8.pm","leader_sync5_2.pm","leader_sync5_3.pm","leader_sync5_4.pm","leader_sync5_5.pm","leader_sync5_6.pm","leader_sync5_8.pm","leader_sync6_2.pm","leader_sync6_3.pm","leader_sync6_4.pm","leader_sync6_5.pm","leader_sync6_6.pm"

# script:
# leader_sync:
for ($i = 0; $i -lt $list_of_leader_sync_files.Length; $i++){
    $this_file = $list_of_leader_sync_files[$i];
    prism.bat "$leader_sync\$this_file" -exportmodel model.sta,tra,lab,trew,rew
    if (-not $?){
        Write-Output "Last execution did not finish successfully! Aborting"
        Pop-Location; exit;
    }
    gc .\instructions.mca
    ..\Release\MC_Analyzer.exe --instructions .\instructions.mca --json-log ".\leader_sync\log$i.json"
    if (-not $?){
        Write-Output "Last execution did not finish successfully! Aborting"
        Pop-Location; exit;
    }
}
# herman:
for ($i = 3; $i -lt 16; $i = $i + 2){
    gc .\instructions.mca
    $instructions=@'
reset_mc>13>2>2
generate_herman>13>
'@
    $instructions += "$i"
    $instructions += @'
>20
print_mc>13
calc_variance>13>0>20>1>0>1
write_state_decorations>13>./output.decos
'@
    Write-Output $instructions | ..\Release\MC_Analyzer.exe --json-log ".\herman\log$i.json"
    if (-not $?){
        Write-Output "Last execution did not finish successfully! Aborting"
        Pop-Location; exit;
    }
}


Pop-Location