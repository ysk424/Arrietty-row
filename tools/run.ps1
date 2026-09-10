[CmdletBinding()]
param([switch]$Demo,[ValidateRange(0,1)][double]$Volume=0.8,[string]$EngineRoot='C:/Program Files/Epic Games/UE_5.8')
$ErrorActionPreference='Stop'
$repo=Split-Path -Parent $PSScriptRoot
$project=Join-Path $repo 'unreal/ArriettyRow/ArriettyRow.uproject'
$editor=Join-Path $EngineRoot 'Engine/Binaries/Win64/UnrealEditor.exe'
$config=Join-Path $repo 'settings.local.json'
if(-not (Test-Path -LiteralPath $editor -PathType Leaf)) { throw "UE editor missing: $editor" }
$logDir=Join-Path $repo 'logs'
New-Item -ItemType Directory -Force -Path $logDir | Out-Null
$log=Join-Path $logDir 'training.log'
# Empty the single normal-run log before UE opens it. UE does not create a
# timestamped backup for an empty file. Exclusive access fails if still in use.
# Session CSV history and deliberately archived diagnostics are separate.
$logStream=[IO.File]::Open($log,[IO.FileMode]::Create,[IO.FileAccess]::Write,[IO.FileShare]::None)
$logStream.Dispose()
$arguments=@($project,'/Game/Row/Maps/BledRow','-game',"-RowSettings=$config",'-nosplash','-windowed','-ResX=1600','-ResY=900')
$arguments+='-abslog='+$log
if($Demo) { $arguments+=@('-RowDemo','-nohmd') } else { $arguments+='-vr' }
$arguments+='-RowVolume='+$Volume.ToString([Globalization.CultureInfo]::InvariantCulture)
& $editor @arguments
