[CmdletBinding()]
param([switch]$Chase,[switch]$Setup,[switch]$Water,[switch]$Straight,
    [ValidatePattern('^[a-zA-Z0-9_-]+$')][string]$Name,
    [string]$EngineRoot='C:/Program Files/Epic Games/UE_5.8')
$ErrorActionPreference='Stop'
$repo=Split-Path -Parent $PSScriptRoot
$project=Join-Path $repo 'unreal/ArriettyRow/ArriettyRow.uproject'
if(-not $Name) { $Name=if($Setup){'row-setup'}elseif($Water){$(if($Straight){'row-kelvin-straight'}else{'row-kelvin-turn'})}elseif($Chase){'row-wake'}else{'row-view'} }
$output=Join-Path $repo ('artifacts/'+$name+'.png')
$log=Join-Path $repo ('logs/'+$name+'.log')
New-Item -ItemType Directory -Force -Path (Split-Path $output) | Out-Null
# Start-Process needs quoted individual path arguments on Windows.
$args=@(('"'+$project+'"'),'/Game/Row/Maps/BledRow','-game','-RowDemo','-nohmd','-RenderOffscreen',
    '-unattended','-nop4','-nosplash','-nosound','-windowed','-ForceRes','-ResX=1600','-ResY=1000',
    ('-RowScreenshotAt='+$(if($Setup){'10.5'}elseif($Water){'52'}else{'24'})),('-RowQuitAfter='+$(if($Setup){'12'}elseif($Water){'56'}else{'28'})),('-RowScreenshotPath="'+$output+'"'),('-abslog="'+$log+'"'),
    '-ExecCmds="t.MaxFPS 60,t.IdleWhenNotForeground 0,r.SetRes 1600x1000w"')
if($Chase) { $args+='-RowChase' }
if($Water) { $args+='-RowWaterView' }
if($Straight) { $args+='-RowDemoStraight' }
$process=Start-Process -FilePath (Join-Path $EngineRoot 'Engine/Binaries/Win64/UnrealEditor.exe') -ArgumentList $args -WindowStyle Hidden -PassThru
Write-Output "Preview started (PID $($process.Id)): $output"
