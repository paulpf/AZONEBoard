#!/usr/bin/env bash
# ============================================================
# setup_secrets.sh
# Creates secret/config header files outside the repository
# so they are never accidentally committed to version control.
#
# Generated files in an external base directory:
#   <base>/_secrets/WifiSecret.h
#   <base>/_secrets/MqttSecret.h
#   <base>/_secrets/OtaSecret.h
#   <base>/_config/MqttConfig.h
#
# Base directory rule:
#   always the parent directory of the repo root
# ============================================================

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
SECRETS_BASE="$(cd "$REPO_ROOT/.." && pwd)"

SECRETS_DIR="$SECRETS_BASE/_secrets"
CONFIGS_DIR="$SECRETS_BASE/_config"

echo ""
echo "=== AZONEBoard - Secret Setup ==="
echo "Repo root    : $REPO_ROOT"
echo "Secrets dir  : $SECRETS_DIR"
echo "Configs dir  : $CONFIGS_DIR"
echo ""

for dir in "$SECRETS_DIR" "$CONFIGS_DIR"; do
    if [ ! -d "$dir" ]; then
        mkdir -p "$dir"
        echo "[CREATED] Directory: $dir"
    else
        echo "[EXISTS]  Directory: $dir"
    fi
done

created=0
skipped=0

write_file_if_missing() {
    local path="$1"
    local content="$2"
    if [ -f "$path" ]; then
        skipped=$((skipped + 1))
        echo "[SKIPPED] $path (already exists, not overwritten)"
    else
        printf '%s\n' "$content" > "$path"
        created=$((created + 1))
        echo "[CREATED] $path"
    fi
}

WIFI_SECRET='#ifndef WIFI_SECRET_H
#define WIFI_SECRET_H

// TODO: Replace with your WiFi credentials
const char *ssid = "Your_WiFi_SSID";
const char *password = "Your_WiFi_Password";

#endif // WIFI_SECRET_H'

MQTT_SECRET='#ifndef MQTT_SECRET_H
#define MQTT_SECRET_H

// TODO: Replace with your MQTT broker credentials
// Leave empty strings if the broker requires no authentication.
const char *mqtt_user = "";
const char *mqtt_password = "";

#endif // MQTT_SECRET_H'

OTA_SECRET='#ifndef OTA_SECRET_H
#define OTA_SECRET_H

// TODO: Set a strong password for OTA updates.
// Leave empty to keep OTA disabled (fail-closed default, see OtaManager).
const char *ota_password = "";

#endif // OTA_SECRET_H'

MQTT_CONFIG='#ifndef MQTT_CONFIG_H
#define MQTT_CONFIG_H

// TODO: Replace with the IP address / hostname of your MQTT broker
const char *mqtt_server = "192.168.x.x";
const int mqtt_port = 1884;

#endif // MQTT_CONFIG_H'

write_file_if_missing "$SECRETS_DIR/WifiSecret.h"  "$WIFI_SECRET"
write_file_if_missing "$SECRETS_DIR/MqttSecret.h"  "$MQTT_SECRET"
write_file_if_missing "$SECRETS_DIR/OtaSecret.h"   "$OTA_SECRET"
write_file_if_missing "$CONFIGS_DIR/MqttConfig.h"  "$MQTT_CONFIG"

echo ""
echo "Summary: created=$created, skipped=$skipped"
echo ""
echo "Done. Fill in all TODO values before building the project."
echo ""
