# build_pal_ab.ps1 ¡ª Build PAL (Mega Drive timing) and PAL2 (NeoGeo timing) bitstreams.
# Requires Quartus 17.x on PATH (quartus_sh).
$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
if (-not (Test-Path (Join-Path $Root "fpga\PAL.qpf"))) {
    $Root = "D:\godot project\240pal\MiSTer_PAL"
}
$Fpga = Join-Path $Root "fpga"
$Other = Join-Path $Root "_Other"
New-Item -ItemType Directory -Force -Path $Other | Out-Null

function Ensure-PAL2Project {
    $qpfSrc = Join-Path $Fpga "PAL.qpf"
    $qsfSrc = Join-Path $Fpga "PAL.qsf"
    $qpfDst = Join-Path $Fpga "PAL2.qpf"
    $qsfDst = Join-Path $Fpga "PAL2.qsf"
    $macro = 'set_global_assignment -name VERILOG_MACRO "PAL_VIDEO_NEOGEO=1"'

    Copy-Item -Force $qpfSrc $qpfDst
    (Get-Content $qpfDst -Raw) -replace 'PROJECT_REVISION = "PAL"', 'PROJECT_REVISION = "PAL2"' | Set-Content $qpfDst -NoNewline

    Copy-Item -Force $qsfSrc $qsfDst
    $qsfLines = Get-Content $qsfDst
    if ($qsfLines -notcontains $macro) {
        $insertAt = 0
        for ($i = 0; $i -lt $qsfLines.Count; $i++) {
            if ($qsfLines[$i] -match "VERILOG_MACRO") { $insertAt = $i + 1 }
        }
        $before = $qsfLines[0..($insertAt - 1)]
        $after = @()
        if ($insertAt -lt $qsfLines.Count) { $after = $qsfLines[$insertAt..($qsfLines.Count - 1)] }
        $qsfLines = $before + $macro + $after
        $utf8NoBom = New-Object System.Text.UTF8Encoding $false
        [System.IO.File]::WriteAllLines($qsfDst, $qsfLines, $utf8NoBom)
    }
}

function Invoke-QuartusCompile {
    param([string]$ProjectBase)
    Push-Location $Fpga
    try {
        Write-Host "==> quartus_sh --flow compile $ProjectBase -c $ProjectBase"
        & quartus_sh --flow compile $ProjectBase -c $ProjectBase
        if ($LASTEXITCODE -ne 0) { throw "Quartus compile failed for $ProjectBase (exit $LASTEXITCODE)" }
    }
    finally {
        Pop-Location
    }
}

function Copy-Rbf {
    param([string]$Revision, [string]$DestName)
    $src = Join-Path $Fpga "output_files\$Revision.rbf"
    if (-not (Test-Path $src)) { throw "Missing $src" }
    $dst = Join-Path $Other $DestName
    Copy-Item -Force $src $dst
    Write-Host "Copied $src -> $dst"
}

Write-Host "MiSTer_PAL root: $Root"
Write-Host "FPGA dir: $Fpga"

# A) PAL ¡ª default Mega Drive / Genesis PLL path (no PAL_VIDEO_NEOGEO)
Invoke-QuartusCompile -ProjectBase "PAL"
Copy-Rbf -Revision "PAL" -DestName "PAL.rbf"

# B) PAL2 ¡ª NeoGeo video timing (PAL_VIDEO_NEOGEO=1 in PAL2.qsf)
Ensure-PAL2Project
Invoke-QuartusCompile -ProjectBase "PAL2"
Copy-Rbf -Revision "PAL2" -DestName "PAL2.rbf"

Write-Host "Done: _Other/PAL.rbf and _Other/PAL2.rbf"
