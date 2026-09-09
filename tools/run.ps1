[CmdletBinding()]
param([switch]$Demo,[string]$EngineRoot='C:/Program Files/Epic Games/UE_5.8')
$repo=Split-Path -Parent $PSScriptRoot
$project=Join-Path $repo 'unreal/ArriettyRow/ArriettyRow.uproject'
$editor=Join-Path $EngineRoot 'Engine/Binaries/Win64/UnrealEditor.exe'
$config=Join-Path $repo 'settings.local.json'
$arguments=@($project,'/Game/Row/Maps/BledRow','-game',"-RowSettings=$config",'-nosplash','-windowed','-ResX=1600','-ResY=900')
if($Demo) { $arguments+=@('-RowDemo','-nohmd') } else { $arguments+='-vr' }
& $editor @arguments
