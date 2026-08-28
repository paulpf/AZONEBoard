# Migrationsplan: ESP8266_template-Patterns → AZONEBoard

Stand: 2026-08-27
Quellen: `Z:\ESP8266_template` (Referenzarchitektur) und `Z:\AZONEBoard` (Zielprojekt)

## 1. Ziel

AZONEBoard soll die im Template etablierten Architektur- und Betriebsmuster übernehmen:
Schichtentrennung (app/config/contracts/domain/infrastructure), nicht-blockierendes Setup,
robuste WiFi-Reconnect-Logik, abgesichertes OTA, zentrales Logging, Secrets komplett
außerhalb des Repos sowie native Unit-Tests für die reine Logik. Die fachliche Funktion von
AZONEBoard (drei Sensoren lesen, über Serial/MQTT/Webserver publizieren) bleibt unverändert –
es handelt sich um ein strukturelles Refactoring, kein Feature-Rewrite.

## 2. Muster aus dem Template (Kurzreferenz)

| Muster | Umsetzung im Template | Warum relevant für AZONEBoard |
|---|---|---|
| Schichtenarchitektur | `src/app`, `src/config`, `src/contracts`, `src/domain`, `src/infrastructure` | AZONEBoard hat aktuell `_infra`, `_interfaces`, `_structures`, aber keine `domain`-Schicht und kein zentrales `config` |
| Composition Root | `Bootstrap` baut alle Manager und injiziert sie per Konstruktor in `Application` | AZONEBoard verdrahtet aktuell alles als globale Objekte direkt in `main.cpp` |
| Nicht-blockierendes Setup | `Application::handleStartup()` State Machine (`WAITING_FOR_WIFI` → `RUNNING`), Timeout statt Endlos-Warten | `EspWifiClient::setup()` blockiert aktuell mit `while(...) delay(500)` ohne Timeout – bereits im Review als Robustheitsproblem markiert |
| WiFi-Reconnect | `WifiManager` als State Machine (`DISCONNECTED/CONNECTING/CONNECTED`) + `ReconnectPolicy::computeDelayMs()` (reine, testbare Funktion) mit Exponential Backoff + Jitter, begrenzte Retry-Anzahl | AZONEBoard hat aktuell keinerlei Reconnect-Logik nach dem initialen Connect |
| OTA-Absicherung | `OtaManager` fail-closed: ohne `OTA_PASSWORD` bleibt OTA deaktiviert, außer explizit `OTA_ALLOW_INSECURE_NO_PASSWORD` gesetzt | AZONEBoards `OtaManager` hat aktuell **keinen** Passwortschutz – jeder im Netz kann Firmware flashen |
| Logging | `Trace` mit Log-Leveln (`TRACE…ERROR`), `logf()` mit festem Stack-Buffer statt String-Konkatenation | AZONEBoard nutzt verstreute `Serial.print/println`-Aufrufe in praktisch jeder Klasse |
| Zentrale Konfiguration | `config.h` (Timings, Feature-Flags, Device-Name), `systemconfig.h` als injizierbare Struct | AZONEBoard hat Magic Numbers verteilt in `main.cpp` (`5000`, `60000*5`), `EepromManager.cpp` (`512`, `1000000`), `MqttConfig.h` (Port `1884`) |
| Contracts (Interfaces) | `IWifiConnectivity`, `IOtaLoopControl` – schmale, fokussierte Interfaces | AZONEBoard hat bereits `IPublisher` (gutes Vorbild), aber keine Interfaces für Wifi/OTA |
| Secrets außerhalb des Repos | `-I ../_secrets` als Build-Flag statt `#ifdef USE_PRIVATE_SECRET`-Verzweigung im Code; `setup_secrets.ps1/.sh` legt Stub-Dateien nur an, wenn sie fehlen | AZONEBoard checkt `WifiSecret.h`/`MqttSecret.h` mit Platzhaltern direkt im Repo ein – Risiko, dass echte Zugangsdaten versehentlich committet werden (im Review bereits angemerkt) |
| OTA-Upload ohne feste IP | `upload_ota.ps1/.bat` fragt IP interaktiv ab, merkt sich letzte IP, liest Passwort aus `OtaSecret.h`, baut temporäres `platformio.ini`-Override | AZONEBoard hat die OTA-IP `192.168.178.45` fest in `platformio.ini` einprogrammiert |
| Native Unit-Tests | `env:native` + Unity, testet z. B. `ReconnectPolicy` ohne Hardware | AZONEBoard hat nur eine leere `test/README`, keine Tests |
| CI in zwei Jobs | `firmware-build` (mit Stub-Secrets) und `native-tests` getrennt | AZONEBoards CI baut nur die Firmware, ohne Tests |

## 3. Zielstruktur für AZONEBoard (Mapping alt → neu)

```
src/
├── app/
│   ├── main.cpp                 (ersetzt aktuelles main.cpp – nur noch setup()/loop() Delegation)
│   ├── bootstrap.h/.cpp         (neu – Composition Root)
│   └── application.h/.cpp       (neu – orchestriert Wifi/OTA/Sensor-Timing/Publisher)
├── config/
│   ├── config.h                 (neu – zentrale Timings/Flags/DEVICE_NAME, ersetzt Magic Numbers)
│   ├── global_defines.h         (neu – gemeinsame Includes)
│   └── systemconfig.h           (neu – injizierbare Timing-Struct, inkl. EEPROM-Intervall)
├── contracts/
│   ├── iwificonnectivity.h      (neu)
│   ├── iotaloopcontrol.h        (neu)
│   └── ipublisher.h             (bestehend, aus _interfaces/IPublisher.h verschoben)
├── domain/
│   ├── reconnectpolicy.h        (neu – 1:1 aus Template übernehmbar)
│   └── intervalpolicy.h         (neu, optional – EEPROM-Bounds-Check aus EepromManager als reine Funktion)
├── infrastructure/
│   ├── wifimanager.h/.cpp       (ersetzt EspWifiClient)
│   ├── otamanager.h/.cpp        (ersetzt bestehenden OtaManager, jetzt mit Passwortschutz)
│   ├── eepromsettings.h/.cpp    (umbenannt aus EepromManager, nutzt Trace)
│   └── trace.h/.cpp             (neu)
├── publishers/
│   ├── serialpublisher.h/.cpp
│   ├── mqttpublisher.h/.cpp
│   └── webserverpublisher.h/.cpp
├── sensors/
│   └── sensormanager.h/.cpp
└── structures/
    ├── sensordata.h
    ├── commondata.h
    └── topicvaluepair.h
```

Hinweis zur Groß-/Kleinschreibung: Das Template nutzt durchgehend Kleinbuchstaben für
Dateinamen, AZONEBoard aktuell PascalCase (`EepromManager.h`). Das ist eine reine
Stilfrage ohne funktionale Relevanz – siehe offene Entscheidung in Abschnitt 6.

## 4. Phasenplan

Jede Phase ist einzeln buildbar/flashbar, damit auf echter Hardware zwischengetestet werden
kann, statt eines riskanten Big-Bang-Rewrites.

### Phase 0 – Vorbereitung (kein Funktionscode)
- Branch `refactor/template-architecture` anlegen.
- `test/`-Ordnerstruktur für `env:native` vorbereiten (wie im Template `test/native/...`).
- `.gitignore` von AZONEBoard um Secrets-Pfade ergänzen bzw. später ersatzlos streichen,
  sobald Secrets komplett ausgelagert sind (siehe Phase 2).
- Aufwand: klein. Risiko: keins.

### Phase 1 – Config- und Logging-Fundament
- `src/config/config.h` anlegen: `SERIAL_BAUD_RATE`, `WATCHDOG_TIMEOUT`,
  `SENSOR_UPDATE_INTERVAL_DEFAULT_MS` (bisher `defaultUpdateSensorDataInterval = 5000`),
  `COMMON_DATA_INTERVAL_MS` (bisher `UpdateCommonDataInterval = 60000*5`),
  `EEPROM_SIZE` (bisher `512`), `EEPROM_INTERVAL_MAX_MS` (bisher `1000000`), `DEVICE_NAME_PREFIX`
  (bisher `"AZ-ONEBoard/"`), `TRACE_LEVEL`.
- `src/config/global_defines.h` und `src/config/systemconfig.h` nach Template-Vorbild anlegen.
- `src/infrastructure/trace.h/.cpp` 1:1 aus dem Template übernehmen.
- Alle `Serial.print(...)`/`Serial.println(...)`-Aufrufe in `SensorManager`, `MqttPublisher`,
  `SerialPublisher`, `WebserverPublisher`, `EspWifiClient`/`WifiManager` durch `Trace::log(...)`
  bzw. `Trace::logf(...)` ersetzen.
- Ergebnis ist bereits eigenständig testbar (Build + Flash), da rein additiv/Ersetzung ohne
  Verhaltensänderung.
- Aufwand: mittel (viele kleine Stellen). Risiko: gering.

### Phase 2 – Secrets & Build-System
- `scripts/setup_secrets.ps1/.sh/.bat` aus dem Template übernehmen und für AZONEBoard anpassen:
  erzeugt `../_secrets/WifiSecret.h`, `../_secrets/MqttSecret.h`, `../_secrets/OtaSecret.h`
  (neu – aktuell hat AZONEBoard keinen OTA-Passwortschutz) und `../_config/MqttConfig.h`.
- `platformio.ini` umstellen: `build_flags = -I src/app -I src/config -I src/contracts
  -I src/domain -I src/infrastructure -I ../_secrets -I ../_config`, dadurch entfällt das
  `#ifdef USE_PRIVATE_SECRET`-Branching in `MqttPublisher.cpp`/`EspWifiClient.cpp` komplett
  (weniger Codepfade, weniger Fehlerquellen).
- Die im Repo eingecheckten `src/_secrets/WifiSecret.h`, `src/Publishers/Mqtt/MqttSecret.h`,
  `Publishers/Mqtt/MqttConfig.h` können danach aus dem Repo entfernt werden (Platzhalter
  verschwinden komplett aus der Versionshistorie ab diesem Commit).
- OTA-Upload-Skript `scripts/upload_ota.ps1` (+ `.bat`) aus dem Template übernehmen und an
  `mqtt_server`/Gerätename anpassen; feste `upload_port = 192.168.178.45` aus `platformio.ini`
  entfernen.
- CI-Workflow um `Create stub secrets`-Schritt (`bash scripts/setup_secrets.sh`) vor dem Build
  ergänzen, analog zum Template.
- Aufwand: mittel. Risiko: mittel (Build-Flag-Änderungen können zunächst Compile-Fehler
  verursachen – nach Phase 2 unbedingt lokal bauen, bevor weitergemacht wird).

### Phase 3 – Domain- und Contracts-Schicht
- `src/domain/reconnectpolicy.h` unverändert aus dem Template übernehmen.
- `src/contracts/iwificonnectivity.h` und `iotaloopcontrol.h` unverändert übernehmen.
- Optional: `src/domain/intervalpolicy.h` – die Bounds-Prüfung aus
  `EepromManager::readUpdateSensorDataInterval` (`dataUpdateTime <= 0 || > 1000000`) als reine
  Funktion `IntervalPolicy::isValid(int value, int min, int max)` auslagern, analog zum
  Vorbild `ReconnectPolicy`. Macht diese Logik nativ testbar, ohne EEPROM-Hardware.
- Aufwand: klein, da größtenteils Copy-Paste aus dem Template. Risiko: keins (noch nicht
  verdrahtet).

### Phase 4 – Infrastruktur: WifiManager & OtaManager
- `EspWifiClient` durch `WifiManager` (Template-Kopie, angepasst an Gerätenamen-Schema
  `AZ-ONEBoard/<MAC>`) ersetzen. Das behebt direkt den im Review gefundenen Blocking-Bug:
  kein `while(...) delay(500)` mehr, sondern State Machine mit Timeout, danach Reconnect mit
  Backoff+Jitter über `ReconnectPolicy`.
- Bestehenden `OtaManager` durch die Template-Version ersetzen (Passwortschutz, fail-closed,
  `isEnabled()`/`isUpdating()`). Erfordert `OTA_PASSWORD` aus `OtaSecret.h` (Phase 2).
- `EepromManager` bleibt funktional, wird aber auf `Trace` statt `Serial.println` umgestellt
  und optional in `infrastructure/` umbenannt in `eepromsettings.h/.cpp` für konsistente
  Benennung.
- Aufwand: mittel–hoch (Kernlogik ändert sich, unbedingt auf echter Hardware testen:
  WLAN trennen/wiederherstellen, OTA mit und ohne Passwort ausprobieren).

### Phase 5 – Application/Bootstrap & main.cpp
- `Bootstrap` als Composition Root anlegen: hält `WifiManager`, `OtaManager`, `EepromManager`,
  `SensorManager`, `SerialPublisher`, `MqttPublisher`, `WebserverPublisher`, `SystemConfig` und
  konstruiert `Application` mit Referenzen darauf (Dependency Injection wie im Template).
- `Application::setup()`/`loop()` übernimmt die aktuelle Logik aus `main.cpp`, aber als
  State Machine analog `handleStartup()`: WiFi verbinden (nicht blockierend) → sobald
  verbunden: OTA aufsetzen, `publishAtSetup()` (einmalige Common-Data-Publikation) → danach
  regulärer Loop-Betrieb.
- Im regulären Loop zwei Zeitfenster analog zum Template-Statusprint ergänzen:
  - Sensor-Update-Intervall (bestehend, aus EEPROM/Config)
  - **Common-Data-Intervall** (`COMMON_DATA_INTERVAL_MS`), das im aktuellen `main.cpp`
    deklariert aber nie ausgewertet wird → behebt den im Review gefundenen Bug, dass
    IP-Adresse/Intervall nur einmal beim Boot publiziert werden.
- `main.cpp` wird auf das Template-Minimum reduziert (`Bootstrap`, `setup()`, `loop()`).
- Aufwand: mittel–hoch, da hier die gesamte Ablaufsteuerung zusammengeführt wird. Risiko:
  mittel – sorgfältig gegen die bisherige `main.cpp`-Logik abgleichen, damit kein
  Sensor-/Publish-Zyklus verloren geht.

### Phase 6 – Publisher- und Sensor-Schicht anpassen
- `SerialPublisher`, `MqttPublisher`, `WebserverPublisher` bekommen `Trace` statt
  `Serial.print` (aus Phase 1) und beziehen Gerätename/Config über `SystemConfig`/`config.h`
  statt lokaler Literale.
- `WebserverPublisher.cpp`: das verwaiste `#include "config.h"` (im vorherigen Review als
  fragiler Ghost-Include markiert) wird entweder entfernt oder bewusst auf das neue,
  tatsächlich existierende `src/config/config.h` gerichtet, falls dort benötigte Konstanten
  liegen.
- Optional (nicht Teil der Template-Übernahme, aber naheliegend, da Sensors/ ohnehin
  angefasst wird): doppelte Sensor-Reads in `SensorManager` beheben – `readTemperature()`
  und `readHumidity()` rufen aktuell beide unabhängig `sht30.get()` auf, analog bei
  `IAQmeasure()`/`IAQmeasureRaw()`. Empfehlung: einmal pro Zyklus lesen und Ergebnis cachen.
- Aufwand: klein–mittel. Risiko: gering.

### Phase 7 – Tests & CI
- `env:native` in `platformio.ini` ergänzen (`platform = native`, `test_framework = unity`).
- `test/native/test_reconnect_policy/test_main.cpp` aus dem Template übernehmen.
- Eigene Tests ergänzen, z. B. für `IntervalPolicy` (Phase 3) oder für reine
  Sensor-Fehleraggregation, falls diese Logik ebenfalls in eine `domain/`-Funktion
  extrahiert wird.
- CI-Workflow (`.github/workflows/c-cpp.yml`) in zwei Jobs aufteilen: `firmware-build`
  (inkl. `Create stub secrets`-Schritt) und `native-tests`, analog zum Template
  (`.github/workflows/ci.yml`). Dabei auch den bestehenden Copy-Paste-Fehler im
  Step-Namen (beide Steps heißen `"Build environment esp12e-usb"`) korrigieren.
- Aufwand: klein–mittel. Risiko: keins (rein additiv).

### Phase 8 – Doku, Cleanup, Hardware-Verifikation
- README.md aktualisieren: neue Projektstruktur, neuer Secrets-Workflow
  (`scripts/setup_secrets.ps1`), neuer OTA-Workflow (`scripts/upload_ota.ps1`), Hinweis auf
  `pio test -e native`.
- Alte Dateien/Ordner entfernen, die durch die Migration ersetzt wurden (`_infra/`,
  `_interfaces/`, `_secrets/` im Repo, alte `EspWifiClient.*`).
- Vollständiger End-to-End-Test auf echter Hardware: USB-Flash, WLAN-Verbindungsabbruch
  simulieren (Router kurz aus/an), OTA-Update mit Passwort, MQTT- und Webserver-Ausgabe
  prüfen, EEPROM-Wert über MQTT ändern und Neustart-Persistenz verifizieren.
- Aufwand: klein–mittel.

## 5. Aus dem Review bereits bekannte Probleme, die diese Migration mit abdeckt

- **Blockierendes WiFi-Setup ohne Timeout** → gelöst durch `WifiManager` (Phase 4).
- **Fehlender periodischer `publishCommonData()`-Aufruf** (`UpdateCommonDataInterval`
  deklariert, aber nie ausgewertet) → gelöst durch Application-Loop-Timing (Phase 5).
- **Verwaister `#include "config.h"`** in `WebserverPublisher.cpp` → aufgelöst, sobald ein
  echtes `config.h` existiert bzw. Include entfernt wird (Phase 6).
- **OTA ohne Passwortschutz** → gelöst durch `OtaManager` fail-closed (Phase 4).
- **Secrets potenziell im Repo committbar** → gelöst durch vollständige Auslagerung nach
  `../_secrets` (Phase 2).
- **Keine Tests** → gelöst durch `env:native` + Unity (Phase 7).
- Nicht direkt durch das Template abgedeckt, aber bei der Gelegenheit sinnvoll zu beheben:
  doppelte Sensor-Reads (Phase 6, optional), `sensorManager.setup()`-Rückgabewert wird
  aktuell ignoriert (in Application::setup() künftig auswerten und in Trace loggen).

## 6. Offene Entscheidungen für dich

- **Dateinamens-Konvention**: PascalCase (bisher AZONEBoard) vs. lowercase (Template) –
  Empfehlung: beim Umbenennen bleiben, aber nur, wenn dir Konsistenz mit dem Template wichtig
  ist; funktional irrelevant.
- **MQTT-Board**: AZONEBoard nutzt `esp12e` (ESP8266), Template nutzt `d1_mini` (ebenfalls
  ESP8266) – beide Boards sind kompatibel, `WifiManager`/`OtaManager` sind 1:1 übertragbar,
  die `platformio.ini`-Envs müssen aber auf `board = esp12e` bleiben.
- **`WIFI_RESTART_ON_RECONNECT_FAILURE`**: Template-Default ist `false` (Gerät bleibt für
  Diagnose am Leben statt Neustart). Für ein unbeaufsichtigtes IoT-Board evtl. `true`
  sinnvoller – bitte bewusst entscheiden.
- **Umfang von Phase 6 (Sensor-Read-Fix)**: optional, nicht Teil der eigentlichen
  Muster-Übernahme, aber im selben Umbau günstig mitzunehmen.

## 7. Empfohlene Reihenfolge / Aufwand

Empfehlung: strikt sequenziell 0 → 8 abarbeiten, nach jeder Phase (mind. nach 2, 4 und 5)
real flashen und testen, da WiFi/OTA/EEPROM-Verhalten nicht sinnvoll ohne Hardware
verifizierbar ist. Phasen 1, 3, 7 sind risikoarm und gut parallelisierbar/vorab erledigbar.
Gesamtaufwand grob geschätzt: 2-3 fokussierte Arbeitssessions für ein Hobby-Projekt dieser
Größe, plus Zeit für Hardware-Tests zwischen den Phasen.
