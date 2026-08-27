param(
    [string]$IpAddress
)

$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path "$PSScriptRoot\.."
$secretsBase = Resolve-Path "$repoRoot\.."
$secretsDir = Join-Path $secretsBase "_secrets"
$lastIpFile = Join-Path $secretsDir "last_ota_ip.txt"
$otaSecretFile = Join-Path $secretsDir "OtaSecret.h"

if (-not (Test-Path $secretsDir)) {
    New-Item -ItemType Directory -Path $secretsDir | Out-Null
}

$lastIp = ""
if (Test-Path $lastIpFile) {
    $lastIp = (Get-Content $lastIpFile -ErrorAction SilentlyContinue | Select-Object -First 1).Trim()
}

if ([string]::IsNullOrWhiteSpace($IpAddress)) {
    $defaultIp = $lastIp
    if ([string]::IsNullOrWhiteSpace($defaultIp)) { $defaultIp = "" }

    try {
        Add-Type -AssemblyName Microsoft.VisualBasic | Out-Null
        $msg = "AZ-ONEBoard IP-Adresse fuer OTA Upload eingeben"
        $title = "AZONEBoard OTA Upload"
        $inputIp = [Microsoft.VisualBasic.Interaction]::InputBox($msg, $title, $defaultIp)

        if ([string]::IsNullOrWhiteSpace($inputIp)) {
            if (-not [string]::IsNullOrWhiteSpace($lastIp)) {
                $IpAddress = $lastIp
            }
        }
        else {
            $IpAddress = $inputIp
        }
    }
    catch {
        if ([string]::IsNullOrWhiteSpace($lastIp)) {
            $IpAddress = Read-Host "AZ-ONEBoard IP-Adresse fuer OTA Upload eingeben"
        }
        else {
            $inputIp = Read-Host "AZ-ONEBoard IP-Adresse fuer OTA Upload eingeben [$lastIp]"
            if ([string]::IsNullOrWhiteSpace($inputIp)) {
                $IpAddress = $lastIp
            }
            else {
                $IpAddress = $inputIp
            }
        }
    }
}

if ([string]::IsNullOrWhiteSpace($IpAddress)) {
    Write-Error "Keine IP-Adresse angegeben. Abbruch."
    exit 1
}

# Read OTA password from external secret header (const char *ota_password = "...";)
$otaPassword = ""
if (Test-Path $otaSecretFile) {
    $otaSecretContent = Get-Content $otaSecretFile -Raw -ErrorAction SilentlyContinue
    if ($otaSecretContent -match 'ota_password\s*=\s*"([^"]*)"') {
        $otaPassword = $matches[1]
    }
}

Set-Content -Path $lastIpFile -Value $IpAddress -Encoding UTF8

Write-Host "Starte OTA-Upload zu $IpAddress ..." -ForegroundColor Cyan
Push-Location $repoRoot
$tempProjectConf = $null
try {
    $targetEnv = 'esp12e-ota'
    $pioArgs = @('run', '--upload-port', "$IpAddress", '-t', 'upload')
    if (-not [string]::IsNullOrWhiteSpace($otaPassword)) {
        $baseProjectConf = Join-Path $repoRoot "platformio.ini"
        $baseContent = Get-Content $baseProjectConf -Raw

        $tempProjectConf = Join-Path $env:TEMP ("platformio_ota_{0}.ini" -f [guid]::NewGuid().ToString("N"))
        $runtimeOverride = @"

[env:esp12e-ota-runtime]
extends = env:esp12e-ota
upload_flags =
  --auth=$otaPassword
"@
        $utf8NoBom = New-Object System.Text.UTF8Encoding($false)
        [System.IO.File]::WriteAllText($tempProjectConf, ($baseContent + $runtimeOverride), $utf8NoBom)
        $targetEnv = 'esp12e-ota-runtime'
        $pioArgs += @('--project-conf', $tempProjectConf)
    }
    else {
        Write-Host "Hinweis: ota_password ist leer. Upload ohne Auth (nur falls OTA_ALLOW_INSECURE_NO_PASSWORD gesetzt ist)." -ForegroundColor Yellow
    }

    $pioArgs += @('-e', $targetEnv)
    & $env:USERPROFILE\.platformio\penv\Scripts\pio.exe @pioArgs
}
finally {
    if (-not [string]::IsNullOrWhiteSpace($tempProjectConf) -and (Test-Path $tempProjectConf)) {
        Remove-Item $tempProjectConf -Force -ErrorAction SilentlyContinue
    }
    Pop-Location
}
