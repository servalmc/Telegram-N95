# Renders Symgram screens at N95 client resolution without a device.
#
# This is NOT a camera shot of the phone: it follows the layout in
# src/SymgramAppView.cpp so the README can show the current UI.
# Russian copy lives in UTF-8 files (preview-copy.txt, mock-chats.txt, .rls)
# so this script stays ASCII and Windows PowerShell can parse it.
#
#   powershell -File tools\preview-ui.ps1 -Screen signin
#   powershell -File tools\preview-ui.ps1 -Screen chats
#   powershell -File tools\preview-ui.ps1 -Screen groups
#   powershell -File tools\preview-ui.ps1 -Screen channels
#   powershell -File tools\preview-ui.ps1 -Screen settings
#   powershell -File tools\preview-ui.ps1 -Screen chat
#
# -Empty is an alias for -Screen signin.

param(
    [string]$Rls  = (Join-Path $PSScriptRoot "..\data\Symgram_16.rls"),
    [string]$Mock = (Join-Path $PSScriptRoot "mock-chats.txt"),
    [string]$Copy = (Join-Path $PSScriptRoot "preview-copy.txt"),
    [string]$Out,
    [ValidateSet("signin", "chats", "groups", "channels", "settings", "chat")]
    [string]$Screen = "chats",
    [switch]$Empty
)

Add-Type -AssemblyName System.Drawing

if ($Empty) { $Screen = "signin" }

if (-not $Out) {
    $name = switch ($Screen) {
        "signin"   { "sign-in.png" }
        "chats"    { "chat-list.png" }
        "groups"   { "groups.png" }
        "channels" { "channels.png" }
        "settings" { "settings.png" }
        "chat"     { "chat.png" }
    }
    $Out = Join-Path $PSScriptRoot "..\docs\images\$name"
}

$utf8 = New-Object System.Text.UTF8Encoding $false
$Rls  = [System.IO.Path]::GetFullPath($Rls)
$Mock = [System.IO.Path]::GetFullPath($Mock)
$Copy = [System.IO.Path]::GetFullPath($Copy)
$Out  = [System.IO.Path]::GetFullPath($Out)
New-Item -ItemType Directory -Force -Path ([System.IO.Path]::GetDirectoryName($Out)) | Out-Null

$rlsText = [System.IO.File]::ReadAllText($Rls, $utf8)
$strings = @{}
foreach ($m in [regex]::Matches($rlsText, 'rls_string\s+(\S+)\s+"([^"]*)"')) {
    $strings[$m.Groups[1].Value] = $m.Groups[2].Value
}

$copyMap = @{}
foreach ($line in [System.IO.File]::ReadAllLines($Copy, $utf8)) {
    $line = $line.Trim()
    if (-not $line -or $line.StartsWith("#")) { continue }
    $eq = $line.IndexOf("=")
    if ($eq -lt 1) { continue }
    $copyMap[$line.Substring(0, $eq)] = $line.Substring($eq + 1)
}

$all = @()
foreach ($line in [System.IO.File]::ReadAllLines($Mock, $utf8)) {
    $line = $line.Trim()
    if (-not $line -or $line.StartsWith("#")) { continue }
    $f = $line -split '\|'
    if ($f.Count -lt 5) { continue }
    $all += @{ n = $f[0]; p = $f[1]; t = $f[2]; u = [int]$f[3]; k = [int]$f[4] }
}

$tab = 0
$headerLeft = $copyMap["tab0"]
$statusText = $copyMap["online"]
switch ($Screen) {
    "signin"   { $statusText = $strings["STRING_r_symgram_status_unsigned"]; $headerLeft = "Symgram" }
    "chats"    { $tab = 0; $headerLeft = $copyMap["tab0"] }
    "groups"   { $tab = 1; $headerLeft = $copyMap["tab1"] }
    "channels" { $tab = 2; $headerLeft = $copyMap["tab2"] }
    "settings" { $headerLeft = $copyMap["settings"] }
    "chat"     { $headerLeft = $copyMap["peer"]; $statusText = "" }
}

function ChatInTab($c, $t) {
    if ($t -eq 1) { return ($c.k -eq 2 -or $c.k -eq 4) }
    if ($t -eq 2) { return ($c.k -eq 3) }
    return ($c.k -eq 1 -or $c.k -eq 0)
}

$chats = @($all | Where-Object { ChatInTab $_ $tab })

$W = 240; $H = 270
$brand     = [System.Drawing.Color]::FromArgb(0x22, 0x9E, 0xD9)
$paper     = [System.Drawing.Color]::White
$ink       = [System.Drawing.Color]::Black
$muted     = [System.Drawing.Color]::FromArgb(0x8A, 0x8A, 0x8E)
$rule      = [System.Drawing.Color]::FromArgb(0xEC, 0xEC, 0xEC)
$highlight = [System.Drawing.Color]::FromArgb(0xE3, 0xF1, 0xFB)
$bubbleIn  = [System.Drawing.Color]::FromArgb(0xEE, 0xEE, 0xEE)
$bubbleOut = [System.Drawing.Color]::FromArgb(0xD6, 0xEB, 0xF8)

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
$tabH = $textH + 10

function Brush($c) { New-Object System.Drawing.SolidBrush($c) }

$fmt = New-Object System.Drawing.StringFormat
$fmt.Trimming = [System.Drawing.StringTrimming]::EllipsisCharacter
$fmt.FormatFlags = [System.Drawing.StringFormatFlags]::NoWrap
$fmtC = New-Object System.Drawing.StringFormat
$fmtC.Alignment = [System.Drawing.StringAlignment]::Center
$fmtC.LineAlignment = [System.Drawing.StringAlignment]::Center
$fmtC.Trimming = [System.Drawing.StringTrimming]::EllipsisCharacter
$fmtC.FormatFlags = [System.Drawing.StringFormatFlags]::NoWrap

$g.FillRectangle((Brush $brand), 0, 0, $W, $headerH)
$g.DrawString($headerLeft, $titleFont, (Brush $paper), 6, 5)
if ($statusText) {
    $sw = $g.MeasureString($statusText, $textFont).Width
    $g.DrawString($statusText, $textFont, (Brush $paper), $W - 6 - $sw, 6)
}

function DrawTabs($active) {
    $y = $H - $tabH
    $g.FillRectangle((Brush $rule), 0, $y, $W, $tabH)
    $labels = @($copyMap["tab0"], $copyMap["tab1"], $copyMap["tab2"])
    $cw = [int]($W / 3)
    for ($t = 0; $t -lt 3; $t++) {
        $cell = New-Object System.Drawing.RectangleF(($t * $cw), $y, $cw, $tabH)
        if ($t -eq $active) {
            $g.FillRectangle((Brush $brand), $cell.X, $cell.Y, $cell.Width, $cell.Height)
            $g.DrawString($labels[$t], $textFont, (Brush $paper), $cell, $fmtC)
        } else {
            $g.DrawString($labels[$t], $textFont, (Brush $ink), $cell, $fmtC)
        }
    }
}

function DrawRows($rows) {
    $y = $headerH
    $i = 0
    $limit = $H - $tabH
    foreach ($c in $rows) {
        if ($y + $rowH -gt $limit) { break }
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
        $g.DrawLine((New-Object System.Drawing.Pen($rule)), $textLeft, $y + $rowH - 1, $W, $y + $rowH - 1)
        $y += $rowH
        $i++
    }
}

if ($Screen -eq "signin") {
    $y = $headerH + 12
    $g.DrawString($strings["STRING_r_symgram_signin_title"], $nameFont, (Brush $ink), 8, $y)
    $y += $nameH + 14
    $fieldH = $nameH + 12
    $g.FillRectangle((Brush $rule), 6, $y, $W - 12, $fieldH)
    $g.DrawString($strings["STRING_r_symgram_cc_ru"], $nameFont, (Brush $ink), 14, $y + 6)
    $cw = $g.MeasureString("+7", $nameFont).Width
    $g.DrawString("+7", $nameFont, (Brush $ink), $W - 14 - $cw, $y + 6)
    $y += $fieldH + 8
    $g.FillRectangle((Brush $highlight), 6, $y, $W - 12, $fieldH)
    $g.DrawString("+7 ", $nameFont, (Brush $ink), 14, $y + 6)
    $y += $fieldH + 10
    $g.DrawString($strings["STRING_r_symgram_signin_hint"], $textFont, (Brush $muted), 8, $y)
}
elseif ($Screen -eq "settings") {
    $items = @(
        @{ n = $copyMap["account"]; d = $copyMap["phone"] },
        @{ n = $copyMap["logout"]; d = "" },
        @{ n = $copyMap["cache"]; d = "" },
        @{ n = $copyMap["about"]; d = "" }
    )
    $y = $headerH
    for ($i = 0; $i -lt $items.Count; $i++) {
        if ($i -eq 0) { $g.FillRectangle((Brush $highlight), 0, $y, $W, $rowH) }
        $g.DrawString($items[$i].n, $nameFont, (Brush $ink), 8, $y + 4)
        if ($items[$i].d) {
            $g.DrawString($items[$i].d, $textFont, (Brush $muted), 8, $y + 4 + $nameH + 2)
        }
        $y += $rowH
    }
}
elseif ($Screen -eq "chat") {
    $hintH = ($textH * 2) + 8
    $msgs = @(
        @{ t = $copyMap["msg0"]; out = $false },
        @{ t = $copyMap["msg1"]; out = $true },
        @{ t = $copyMap["msg2"]; out = $false }
    )
    $g.FillRectangle((Brush $rule), 0, ($H - $hintH), $W, $hintH)
    $hintRect = New-Object System.Drawing.RectangleF(6, ($H - $hintH + 2), ($W - 12), ($hintH - 4))
    $fmtW = New-Object System.Drawing.StringFormat
    $g.DrawString($copyMap["hint"], $textFont, (Brush $muted), $hintRect, $fmtW)
    $y = $H - $hintH - 4
    for ($i = $msgs.Count - 1; $i -ge 0; $i--) {
        $h = $textH + 14
        $y -= $h
        if ($msgs[$i].out) {
            $boxX = [int]($W / 4)
            $boxW = $W - 6 - $boxX
        } else {
            $boxX = 6
            $boxW = $W - 12 - [int]($W / 5)
        }
        $col = if ($msgs[$i].out) { $bubbleOut } else { $bubbleIn }
        $g.FillRectangle((Brush $col), $boxX, $y, $boxW, $h - 2)
        $g.DrawString($msgs[$i].t, $textFont, (Brush $ink), $boxX + 6, $y + 4)
    }
}
else {
    DrawRows $chats
    DrawTabs $tab
}

$g.Dispose()
$bmp.Save($Out, [System.Drawing.Imaging.ImageFormat]::Png)
$bmp.Dispose()
"written: $Out"
