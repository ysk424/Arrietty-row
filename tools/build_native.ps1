[CmdletBinding()]
param([switch]$Devices)
$ErrorActionPreference='Stop'
$repo=Split-Path -Parent $PSScriptRoot
$out=Join-Path $repo 'artifacts/native'
New-Item -ItemType Directory -Force -Path $out | Out-Null
$vswhere=Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
$vs=& $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if(-not $vs) { throw 'MSVC x64 tools required' }
$setup=Join-Path $vs 'VC/Auxiliary/Build/vcvars64.bat'
$line='cl /nologo /std:c++20 /EHsc /W4 /WX /O2 /I "'+(Join-Path $repo 'Source')+'" "'+(Join-Path $repo 'tests/core_tests.cpp')+'" /Fe:core_tests.exe'
if($Devices) {
    & (Join-Path $PSScriptRoot 'bootstrap.ps1')
    $sdk=Join-Path $repo 'ThirdParty/OpenVR'
    $line='cl /nologo /std:c++20 /EHsc /W3 /O2 /I "'+(Join-Path $repo 'Source')+'" /I "'+(Join-Path $sdk 'headers')+'" "'+(Join-Path $repo 'Source/RowDevices.cpp')+'" "'+(Join-Path $repo 'tools/device_probe.cpp')+'" /Fe:device_probe.exe /link windowsapp.lib "'+(Join-Path $sdk 'lib/win64/openvr_api.lib')+'"'
    Copy-Item -LiteralPath (Join-Path $sdk 'bin/win64/openvr_api.dll') -Destination $out -Force
}
$bat=Join-Path $out 'build.cmd'
@('@echo off',('call "'+$setup+'" >nul'),$line,'exit /b %errorlevel%') | Set-Content -LiteralPath $bat -Encoding ascii
Push-Location $out
try { & $bat; if($LASTEXITCODE -ne 0) { throw 'Native compilation failed' }; if(-not $Devices) { & './core_tests.exe'; if($LASTEXITCODE -ne 0) { throw 'Core tests failed' } } }
finally { Pop-Location }
