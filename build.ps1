# Build USL Integration for Windows (also used by Linux via Proton/Wine) and Android 64/32,
# then package one .geode with all binaries and install it into the local GD profile.
# Usage: powershell -File build.ps1 [-Configure] [-WindowsOnly]
param([switch]$Configure, [switch]$WindowsOnly)
$ErrorActionPreference = "Stop"
$tools = "C:\Users\wayago\geode-tools"
# SDK version = the "geode" field in mod.json = the loader players run.
$env:GEODE_SDK = "E:\GameDev\geode-sdk"
# Shared dependency cache of the mod workspace, so a build does not clone fmt, json & co. every time.
$env:CPM_SOURCE_CACHE = "E:\GameDev\geode-mods\.cache\cpm"
# Bindings must match the ones the Geode loader was built with (its crash log shows "Bindings Commit").
# Without a pin the SDK clones bindings main, which can renumber enums under a released loader:
# on 2026-10-29 main dropped BoomListType::User, so Level became Achievement and list pages crashed.
$bindingsCommit = "1605f500"
$env:GEODE_BINDINGS_REPO_PATH = "$tools\bindings-$bindingsCommit"
if (-not (Test-Path $env:GEODE_BINDINGS_REPO_PATH)) {
  git clone https://github.com/geode-sdk/bindings.git $env:GEODE_BINDINGS_REPO_PATH
  git -C $env:GEODE_BINDINGS_REPO_PATH checkout $bindingsCommit
  if ($LASTEXITCODE) { exit $LASTEXITCODE }
}
$ndk = Get-ChildItem $tools -Directory -Filter "android-ndk-*" | Select-Object -First 1
$vs = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -all -latest -products * -property installationPath
cmd /c "`"$vs\VC\Auxiliary\Build\vcvars64.bat`" >nul && set" | ForEach-Object { if ($_ -match "^(.*?)=(.*)$") { Set-Item "env:$($matches[1])" $matches[2] } }
# Geode v5 headers (arc futures) crash MSVC 14.44 with C1001, so build with portable clang-cl.
$env:PATH = "$tools;$tools\llvm\bin;" + $env:PATH
Set-Location $PSScriptRoot

function Invoke-Build($dir, [string[]]$configureArgs) {
  if ($Configure -or -not (Test-Path "$dir/build.ninja")) {
    cmake -S . -B $dir -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo @configureArgs
    if ($LASTEXITCODE) { exit $LASTEXITCODE }
  }
  # One job: parallel clang on the Geode bindings can run a 16 GB machine out of page file.
  cmake --build $dir -j 1
  if ($LASTEXITCODE) { exit $LASTEXITCODE }
}

# Codegen is built from the pinned bindings instead of downloading the newest release,
# so the generated headers match the bindings exactly.
Invoke-Build "build" @("-DCMAKE_C_COMPILER=clang-cl", "-DCMAKE_CXX_COMPILER=clang-cl", "-DCMAKE_LINKER=lld-link",
  "-DSKIP_BUILDING_CODEGEN=OFF")
$binaries = @("build/wayago.usl-integration.dll")

if (-not $WindowsOnly) {
  if (-not $ndk) { throw "Android NDK not found in $tools" }
  $toolchain = "$($ndk.FullName)\build\cmake\android.toolchain.cmake"
  foreach ($target in @(@{ Dir = "build-android64"; Abi = "arm64-v8a"; Platform = "Android64" },
                        @{ Dir = "build-android32"; Abi = "armeabi-v7a"; Platform = "Android32" })) {
    # Android builds can't compile the host Codegen with the NDK, so they reuse the Windows one;
    # the bindings only take a cached Codegen that is less than a day old.
    $codegen = "$($target.Dir)/bindings/codegen"
    New-Item -ItemType Directory -Force $codegen | Out-Null
    Copy-Item "build/bindings/codegen/Codegen.exe" $codegen -Force
    (Get-Item "$codegen/Codegen.exe").LastWriteTime = Get-Date
    Invoke-Build $target.Dir @("-DCMAKE_TOOLCHAIN_FILE=$toolchain", "-DANDROID_ABI=$($target.Abi)",
      "-DANDROID_PLATFORM=android-23", "-DANDROID_STL=c++_shared", "-DGEODE_TARGET_PLATFORM=$($target.Platform)",
      "-DGEODE_DONT_INSTALL_MODS=ON")
    $binaries += (Get-ChildItem "$($target.Dir)/*.so" | Select-Object -First 1).FullName
  }
  $out = "dist/wayago.usl-integration.geode"
  New-Item -ItemType Directory -Force dist | Out-Null
  $binArgs = $binaries | ForEach-Object { "--binary"; $_ }
  geode package new . @binArgs --output $out --install
  if ($LASTEXITCODE) { exit $LASTEXITCODE }
  Write-Host "Packaged $out"
}
