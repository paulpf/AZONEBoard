# ============================================================
# setup_secrets.ps1
# Creates secret/config header files outside the repository
# so they are never accidentally committed to version control.
#
# Generated files in an external base directory:
#   <base>\_secrets\WifiSecret.h
#   <base>\_secrets\MqttSecret.h
#   <base>\_secrets\OtaSecret.h
#   <base>\_config\MqttConfig.h
#
# Base directory rule:
# - always parent directory of repo root
# ============================================================

$repoRoot = Resolve-Path "$PSScriptRoot\.."
$secretsBase = Resolve-Path "$repoRoot\.."

$secretsDir = Join-Path $secretsBase "_secrets"
$configsDir = Join-Path $secretsBase "_config"

Write-Host ""
Write-Host "=== AZONEBoard - Secret Setup ===" -ForegroundColor Cyan
Write-Host "Repo root    : $repoRoot"
Write-Host "Secrets dir  : $secretsDir"
Write-Host "Configs dir  : $configsDir"
Write-Host ""

foreach ($dir in @($secretsDir, $configsDir)) {
    if (-not (Test-Path $dir)) {
        New-Item -ItemType Directory -Path $dir | Out-Null
        Write-Host "[CREATED] Directory: $dir" -ForegroundColor Green
    }
    else {
        Write-Host "[EXISTS]  Directory: $dir" -ForegroundColor DarkGray
    }
}

function Write-FileIfMissing([string]$path, [string[]]$lines, [ref]$created, [ref]$skipped) {
    if (Test-Path $path) {
        $skipped.Value++
        Write-Host "[SKIPPED] $path (already exists, not overwritten)" -ForegroundColor DarkGray
        return
    }

    $lines | Set-Content -Path $path -Encoding UTF8
    $created.Value++
    Write-Host "[CREATED] $path" -ForegroundColor Green
}

$wifiLines = @(
    '#ifndef WIFI_SECRET_H',
    '#define WIFI_SECRET_H',
    '',
    '// TODO: Replace with your WiFi credentials',
    '#define WIFI_SSID "Your_WiFi_SSID"',
    '#define WIFI_PWD  "Your_WiFi_Password"',
    '',
    '#endif // WIFI_SECRET_H'
)

$mqttSecretLines = @(
    '#ifndef MQTT_SECRET_H',
    '#define MQTT_SECRET_H',
    '',
    '// TODO: Replace with your MQTT broker credentials',
    '// Leave empty strings if the broker requires no authentication.',
    '#define MQTT_USER "your_mqtt_username"',
    '#define MQTT_PWD  "your_mqtt_password"',
    '',
    '#endif // MQTT_SECRET_H'
)

$otaLines = @(
    '#ifndef OTA_SECRET_H',
    '#define OTA_SECRET_H',
    '',
    '// TODO: Set a strong password for OTA updates.',
    '// Leave empty to keep OTA disabled (fail-closed default, see OtaManager).',
    '#define OTA_PASSWORD ""',
    '',
    '#endif // OTA_SECRET_H'
)

$mqttConfigLines = @(
    '#ifndef MQTT_CONFIG_H',
    '#define MQTT_CONFIG_H',
    '',
    '// TODO: Replace with the IP address / hostname of your MQTT broker',
    '#define MQTT_SERVER_IP   "192.168.x.x"',
    '#define MQTT_SERVER_PORT 1883',
    '',
    '#endif // MQTT_CONFIG_H'
)

$created = 0
$skipped = 0

Write-FileIfMissing "$secretsDir\WifiSecret.h" $wifiLines       ([ref]$created) ([ref]$skipped)
Write-FileIfMissing "$secretsDir\MqttSecret.h" $mqttSecretLines ([ref]$created) ([ref]$skipped)
Write-FileIfMissing "$secretsDir\OtaSecret.h"  $otaLines        ([ref]$created) ([ref]$skipped)
Write-FileIfMissing "$configsDir\MqttConfig.h" $mqttConfigLines ([ref]$created) ([ref]$skipped)

Write-Host ""
Write-Host "Summary: created=$created, skipped=$skipped" -ForegroundColor Cyan
Write-Host ""
Write-Host "Done. Fill in all TODO values before building the project." -ForegroundColor Green
Write-Host ""
