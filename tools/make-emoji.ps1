# Builds 24-bit colour BMPs and 8-bit soft masks from emoji-data Twemoji PNGs.
#
# Source: img-twitter-64 (Twemoji). Those PNGs are 32bpp with a real alpha
# channel; Apple/Google sets in the same zip also have alpha, but Twemoji
# edges stay clean at 32px on a 240-wide N95 screen.
#
# mifconv /c24,8 looks for <name>_mask_soft.bmp next to <name>.bmp and stores
# the grey values as an 8-bit mask, so anti-aliased edges are kept.

param(
    [string]$ZipPath = "C:\Users\serval\Downloads\emoji-data-16.0.0.zip",
    [string]$OutDir = (Join-Path $PSScriptRoot "..\gfx\emoji"),
    [int]$Size = 32
)

$ErrorActionPreference = "Stop"
Add-Type -AssemblyName System.IO.Compression.FileSystem
Add-Type -AssemblyName System.Drawing

if (-not (Test-Path -LiteralPath $ZipPath)) {
    throw "emoji-data zip not found: $ZipPath"
}

$OutDir = [System.IO.Path]::GetFullPath($OutDir)
New-Item -ItemType Directory -Force -Path $OutDir | Out-Null

# Same order as KEmojiList in SymgramAppView.cpp.
$codes = @(
    "1f600","1f602","1f60a","1f60d","1f609","1f618",
    "1f622","1f62d","1f621","1f44d","1f44e","1f44c",
    "2764","1f525","2b50","1f389","1f44f","1f64f",
    "1f60e","1f914","1f605","1f923","1f49c","2728"
)

$z = [IO.Compression.ZipFile]::OpenRead($ZipPath)

function Find-Png([string]$hex) {
    $names = @(
        "emoji-data-16.0.0/img-twitter-64/$hex.png",
        "emoji-data-16.0.0/img-twitter-64/$hex-fe0f.png"
    )
    foreach ($n in $names) {
        $e = $z.GetEntry($n)
        if ($e) { return $e }
    }
    return $null
}

function New-24([int]$s) {
    New-Object System.Drawing.Bitmap($s, $s, [System.Drawing.Imaging.PixelFormat]::Format24bppRgb)
}

$i = 0
foreach ($hex in $codes) {
    $entry = Find-Png $hex
    if (-not $entry) {
        $z.Dispose()
        throw "missing twitter-64 PNG for $hex"
    }
    $ms = New-Object IO.MemoryStream
    $stream = $entry.Open()
    $stream.CopyTo($ms)
    $stream.Dispose()
    $ms.Position = 0
    $srcImg = [System.Drawing.Image]::FromStream($ms)
    $src = New-Object System.Drawing.Bitmap $srcImg
    $srcImg.Dispose()

    $scaled = New-Object System.Drawing.Bitmap($Size, $Size, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $g = [System.Drawing.Graphics]::FromImage($scaled)
    $g.Clear([System.Drawing.Color]::Transparent)
    $g.CompositingMode = [System.Drawing.Drawing2D.CompositingMode]::SourceCopy
    $g.CompositingQuality = [System.Drawing.Drawing2D.CompositingQuality]::HighQuality
    $g.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
    $g.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality
    $g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::HighQuality
    $g.DrawImage($src, (New-Object System.Drawing.Rectangle(0, 0, $Size, $Size)))
    $g.Dispose()
    $src.Dispose()
    $ms.Dispose()

    $color = New-24 $Size
    $mask = New-24 $Size
    $y = 0
    while ($y -lt $Size) {
        $x = 0
        while ($x -lt $Size) {
            $p = $scaled.GetPixel($x, $y)
            $a = [int]$p.A
            $color.SetPixel($x, $y, [System.Drawing.Color]::FromArgb(255, $p.R, $p.G, $p.B))
            $mask.SetPixel($x, $y, [System.Drawing.Color]::FromArgb(255, $a, $a, $a))
            $x++
        }
        $y++
    }
    $scaled.Dispose()

    $stem = "e{0:d2}" -f $i
    $color.Save((Join-Path $OutDir "$stem.bmp"), [System.Drawing.Imaging.ImageFormat]::Bmp)
    $mask.Save((Join-Path $OutDir "${stem}_mask_soft.bmp"), [System.Drawing.Imaging.ImageFormat]::Bmp)
    $color.Dispose()
    $mask.Dispose()
    Write-Output ("{0}  {1}  {2}" -f $stem, $hex, $entry.Name)
    $i++
}

$z.Dispose()
Write-Output ("wrote $i emoji pairs to $OutDir")
