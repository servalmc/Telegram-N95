# Renders a mock-up of the chat list at N95 resolution.
#
# This is NOT a device screenshot: it re-implements the layout rules from
# src/SymgramAppView.cpp on the desktop so the design can be reviewed without a
# phone. Keep the constants below in step with the C++ side.
#
# Display strings are read from the .rls resource rather than embedded here, so
# there is one source of truth and no dependence on this file's encoding.

param(
    [string]$Rls = (Join-Path $PSScriptRoot "..\data\Symgram_16.rls"),
    [string]$Out = (Join-Path $PSScriptRoot "..\gfx\preview_ui.png")
)

Add-Type -AssemblyName System.Drawing

$Rls = [System.IO.Path]::GetFullPath($Rls)
$Out = [System.IO.Path]::GetFullPath($Out)

# --- strings from the resource ------------------------------------------------
$rlsText = [System.IO.File]::ReadAllText($Rls, [System.Text.Encoding]::UTF8)
$strings = @{}
foreach ($m in [regex]::Matches($rlsText, 'rls_string\s+(\S+)\s+"([^"]*)"')) {
    $strings[$m.Groups[1].Value] = $m.Groups[2].Value
}

$chats = @()
for ($i = 1; $i -le 20; $i++) {
    $key = "STRING_r_symgram_chat_$i"
    if (-not $strings.ContainsKey($key)) { continue }
    $f = $strings[$key] -split '\|'
    if ($f.Count -lt 4) { continue }
    $chats += @{ n = $f[0]; p = $f[1]; t = $f[2]; u = [int]$f[3] }
}

$statusText = $strings["STRING_r_symgram_status_offline"]
$appName = "Symgram"

# --- geometry: the client rect the application actually draws ------------------
$W = 240; $H = 270          # N95 8GB QVGA, less the status pane and softkey strip

$brand     = [System.Drawing.Color]::FromArgb(0x22, 0x9E, 0xD9)
$paper     = [System.Drawing.Color]::White
$ink       = [System.Drawing.Color]::Black
$muted     = [System.Drawing.Color]::FromArgb(0x8A, 0x8A, 0x8E)
$rule      = [System.Drawing.Color]::FromArgb(0xEC, 0xEC, 0xEC)
$highlight = [System.Drawing.Color]::FromArgb(0xE3, 0xF1, 0xFB)

$avatarColours = @(
    [System.Drawing.Color]::FromArgb(0xE1, 0x7B, 0x60),
    [System.Drawing.Color]::FromArgb(0x7B, 0xC8, 0x62),
    [System.Drawing.Color]::FromArgb(0x65, 0xAA, 0xDD),
    [System.Drawing.Color]::FromArgb(0xA6, 0x95, 0xE7),
    [System.Drawing.Color]::FromArgb(0xEE, 0x9C, 0x5A),
    [System.Drawing.Color]::FromArgb(0xE0, 0x7B, 0xA0)
)

$bmp = New-Object System.Drawing.Bitmap($W, $H)
$g = [System.Drawing.Graphics]::FromImage($bmp)
$g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
$g.TextRenderingHint = [System.Drawing.Text.TextRenderingHint]::ClearTypeGridFit
$g.Clear($paper)

$nameFont  = New-Object System.Drawing.Font("Tahoma", 8.5, [System.Drawing.FontStyle]::Bold)
$textFont  = New-Object System.Drawing.Font("Tahoma", 7.5)
$titleFont = New-Object System.Drawing.Font("Tahoma", 8, [System.Drawing.FontStyle]::Bold)

$nameH  = [int]$nameFont.GetHeight($g)
$textH  = [int]$textFont.GetHeight($g)
$titleH = [int]$titleFont.GetHeight($g)

$headerH = $titleH + 12
$rowH = $nameH + $textH + 14

function Brush($c) { New-Object System.Drawing.SolidBrush($c) }

$fmt = New-Object System.Drawing.StringFormat
$fmt.Trimming = [System.Drawing.StringTrimming]::EllipsisCharacter
$fmt.FormatFlags = [System.Drawing.StringFormatFlags]::NoWrap

# --- brand header -------------------------------------------------------------
$g.FillRectangle((Brush $brand), 0, 0, $W, $headerH)
$g.DrawString($appName, $titleFont, (Brush $paper), 6, 5)
$sw = $g.MeasureString($statusText, $textFont).Width
$g.DrawString($statusText, $textFont, (Brush $paper), $W - 6 - $sw, 6)

# --- rows ---------------------------------------------------------------------
$y = $headerH
$i = 0
foreach ($c in $chats) {
    if ($y + $rowH -gt $H) { break }

    if ($i -eq 0) { $g.FillRectangle((Brush $highlight), 0, $y, $W, $rowH) }

    $d = $rowH - 12
    $g.FillEllipse((Brush $avatarColours[$i % 6]), 6, $y + 6, $d, $d)
    $initial = $c.n.Substring(0, 1)
    $isz = $g.MeasureString($initial, $nameFont)
    $g.DrawString($initial, $nameFont, (Brush $paper),
                  6 + ($d - $isz.Width) / 2, $y + 6 + ($d - $isz.Height) / 2)

    $textLeft = 6 + $d + 8
    $rightEdge = $W - 6
    $top = $y + [int](($rowH - $nameH - $textH - 2) / 2)

    $tw = $g.MeasureString($c.t, $textFont).Width
    $g.DrawString($c.t, $textFont, (Brush $muted), $rightEdge - $tw, $top + 1)

    $badgeW = 0
    if ($c.u -gt 0) {
        $cnt = [string]$c.u
        $badgeH = $textH + 4
        $cw = $g.MeasureString($cnt, $textFont).Width
        $badgeW = $cw + $badgeH
        $bx = $rightEdge - $badgeW
        $by = $top + $nameH + 2
        $path = New-Object System.Drawing.Drawing2D.GraphicsPath
        $path.AddArc($bx, $by, $badgeH, $badgeH, 90, 180)
        $path.AddArc($bx + $badgeW - $badgeH, $by, $badgeH, $badgeH, 270, 180)
        $path.CloseFigure()
        $g.FillPath((Brush $brand), $path)
        $g.DrawString($cnt, $textFont, (Brush $paper), $bx + ($badgeW - $cw) / 2, $by + 1)
        $badgeW += 6
    }

    $nameRect = New-Object System.Drawing.RectangleF($textLeft, $top, ($rightEdge - $textLeft - $tw - 6), ($nameH + 2))
    $g.DrawString($c.n, $nameFont, (Brush $ink), $nameRect, $fmt)

    $prevRect = New-Object System.Drawing.RectangleF($textLeft, ($top + $nameH + 2), ($rightEdge - $textLeft - $badgeW), ($textH + 2))
    $g.DrawString($c.p, $textFont, (Brush $muted), $prevRect, $fmt)

    $g.DrawLine((New-Object System.Drawing.Pen($rule)),
                $textLeft, $y + $rowH - 1, $W, $y + $rowH - 1)

    $y += $rowH
    $i++
}

$g.Dispose()
$bmp.Save($Out, [System.Drawing.Imaging.ImageFormat]::Png)
$bmp.Dispose()

"rows rendered: $i of $($chats.Count)"
"written: $Out"
