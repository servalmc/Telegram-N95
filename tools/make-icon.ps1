# Renders the application icon and its transparency mask as bitmaps.
#
# The vector route (SVG -> mifconv) is unavailable: the svgtbinencode.exe shipped
# with this SDK is a WINSCW build that cannot run on Windows, so icons are
# rasterised here instead and fed to mifconv as BMP pairs.

param(
    [int]$Size = 88,
    [string]$OutDir = (Join-Path $PSScriptRoot "..\gfx")
)

Add-Type -AssemblyName System.Drawing

$OutDir = [System.IO.Path]::GetFullPath($OutDir)
New-Item -ItemType Directory -Force -Path $OutDir | Out-Null

$brandBlue = [System.Drawing.Color]::FromArgb(0x22, 0x9E, 0xD9)
$foldBlue  = [System.Drawing.Color]::FromArgb(0xD2, 0xE5, 0xF1)

# Paper plane outlined in a 100x100 space, scaled to the requested size.
$k = $Size / 100.0
function P([double]$x, [double]$y) {
    New-Object System.Drawing.PointF([float]($x * $k), [float]($y * $k))
}

$body = @( (P 21 48), (P 79 25), (P 69 76), (P 47 63), (P 39 73), (P 39 57) )
$fold = @( (P 39 57), (P 79 25), (P 47 63) )

function New-Canvas([int]$s) {
    $bmp = New-Object System.Drawing.Bitmap($s, $s, [System.Drawing.Imaging.PixelFormat]::Format24bppRgb)
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
    return @($bmp, $g)
}

# --- colour icon -------------------------------------------------------------
$c = New-Canvas $Size
$icon = $c[0]; $g = $c[1]
$g.Clear([System.Drawing.Color]::Black)
$g.FillEllipse((New-Object System.Drawing.SolidBrush($brandBlue)), 0, 0, $Size - 1, $Size - 1)
$g.FillPolygon((New-Object System.Drawing.SolidBrush([System.Drawing.Color]::White)), [System.Drawing.PointF[]]$body)
$g.FillPolygon((New-Object System.Drawing.SolidBrush($foldBlue)), [System.Drawing.PointF[]]$fold)
$g.Dispose()
$icon.Save((Join-Path $OutDir "Symgram.bmp"), [System.Drawing.Imaging.ImageFormat]::Bmp)
$icon.Save((Join-Path $OutDir "preview_icon.png"), [System.Drawing.Imaging.ImageFormat]::Png)
$icon.Dispose()

# --- transparency mask: white where the icon is opaque -----------------------
$c = New-Canvas $Size
$mask = $c[0]; $g = $c[1]
$g.Clear([System.Drawing.Color]::Black)
$g.FillEllipse([System.Drawing.Brushes]::White, 0, 0, $Size - 1, $Size - 1)
$g.Dispose()
$mask.Save((Join-Path $OutDir "Symgram_mask_soft.bmp"), [System.Drawing.Imaging.ImageFormat]::Bmp)
$mask.Dispose()

Get-ChildItem $OutDir -Filter "*.bmp" | ForEach-Object { "{0}  {1} bytes" -f $_.Name, $_.Length }
