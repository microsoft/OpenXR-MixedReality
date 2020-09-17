& git submodule update --init

try {
    Push-Location $PSScriptRoot\shared\ext\bgfx
    & ..\bin\windows\genie --with-examples --with-windows=10.0 --with-dynamic-runtime --vs=winstore100 vs2019
    & ..\bin\windows\genie --with-examples --with-windows=10.0 --with-dynamic-runtime vs2019
}
finally {
    Pop-Location
}