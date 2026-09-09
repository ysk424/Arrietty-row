[CmdletBinding()]
param(
    [string]$EngineRoot='C:/Program Files/Epic Games/UE_5.8',
    [string]$LakeProject=(Join-Path $env:USERPROFILE 'Documents/Unreal Projects/LakeBled'),
    [switch]$SkipBuild
)
$ErrorActionPreference='Stop'
$repo=Split-Path -Parent $PSScriptRoot
$project=Join-Path $repo 'unreal/ArriettyRow/ArriettyRow.uproject'
$version=Get-Content -LiteralPath (Join-Path $EngineRoot 'Engine/Build/Build.version') -Raw | ConvertFrom-Json
if($version.MajorVersion -ne 5 -or $version.MinorVersion -ne 8) { throw 'UE 5.8 required' }
& (Join-Path $PSScriptRoot 'bootstrap.ps1')
$logs=Join-Path $repo 'logs'; New-Item -ItemType Directory -Force -Path $logs | Out-Null
if(-not $SkipBuild) {
    & (Join-Path $EngineRoot 'Engine/Build/BatchFiles/Build.bat') ArriettyRowEditor Win64 Development "-Project=$project" -WaitMutex -NoHotReloadFromIDE *> (Join-Path $logs 'ue-build.log')
    if($LASTEXITCODE -ne 0) { throw 'UE compile failed: logs/ue-build.log' }
}
$source=Join-Path $LakeProject 'Content/Worlds/LakeBled'
if(-not (Test-Path -LiteralPath (Join-Path $source 'Maps/LakeBled.umap'))) { throw 'Prepare the Lake Bled source project first; see README.md' }
$destination=Join-Path $repo 'unreal/ArriettyRow/Content/Worlds/LakeBled'
# Read-only source, a new local content copy. No generated assets enter Git.
py -3.13 -c 'import shutil,sys; shutil.copytree(sys.argv[1],sys.argv[2],dirs_exist_ok=True)' $source $destination
if($LASTEXITCODE -ne 0) { throw 'Scenery staging failed' }
$provenance=Join-Path $repo 'unreal/ArriettyRow/Content/Row/Provenance'
New-Item -ItemType Directory -Force -Path $provenance | Out-Null
Copy-Item -LiteralPath (Join-Path $repo 'docs/SCENERY.md') -Destination (Join-Path $provenance 'SCENERY.md') -Force
$geo=Join-Path $LakeProject 'SourceArt/GeographicSources.json'
if(Test-Path -LiteralPath $geo) { Copy-Item -LiteralPath $geo -Destination $provenance -Force }
$editor=Join-Path $EngineRoot 'Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
& $editor $project -run=pythonscript "-script=$(Join-Path $PSScriptRoot 'build_content.py')" -unattended -nop4 -nosplash -nosound -nohmd -NullRHI "-abslog=$(Join-Path $logs 'ue-content.log')" *> (Join-Path $logs 'ue-content-console.log')
if($LASTEXITCODE -ne 0 -or -not (Select-String -LiteralPath (Join-Path $logs 'ue-content.log') -Pattern 'ROW_CONTENT_READY' -Quiet)) { throw 'Content build failed: logs/ue-content.log' }
Write-Output "Ready: $project"
