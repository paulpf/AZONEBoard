// PublishManager.cpp

#ifdef USE_PRIVATE_SECRET
#include "../../_secrets/secret.h"
#else
#include "secret.h"
#endif

#include "PublishManager.h"
#include <Arduino.h>
#include "SensorData.h"
#include "config.h"
#include <ESP8266mDNS.h>

// Initialize the static instance pointer
PublishManager *PublishManager::instance = nullptr;

// Constructor to initialize PublishManager
PublishManager::PublishManager()
{
    instance = this;
}

void PublishManager::setup()
{
    setupWifi();
    setupMqtt();
}

// Method to get device name
String PublishManager::getDeviceName()
{
    deviceName = "AZ-ONEBoard/";
    deviceName += WiFi.macAddress();
    deviceName.replace(":", "");
    return deviceName;
}

// Method to setup wifi
void PublishManager::setupWifi()
{
    espClient = WiFiClient();

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("WiFi connected");

    // Subscribe on callback for changing IP address
    WiFi.onStationModeGotIP(renewIpAddress);

    deviceName = getDeviceName();

    // Print ip address
    ipAddress = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(instance->ipAddress);

    if (MDNS.begin("esp8266"))
    {
        Serial.println("MDNS responder started");
    }
}

// Method to setup webserver
void PublishManager::setupWebserver()
{
    webServer.begin(80);
    webServer.on("/", handleRoot);
    webServer.on("/data", handleData);
    webServer.begin();
    Serial.println("HTTP server started");
}

void PublishManager::handleRoot()
{
    String html = "<html><head><title>Sensor Data of " + instance->deviceName + "</title>";
    html += "<script src='https://cdn.jsdelivr.net/npm/chart.js'></script>";
    html += "<script src='https://cdn.jsdelivr.net/npm/chartjs-adapter-date-fns'></script>";
    html += "<script>";
    html += "var updateInterval = 1000;"; // Default update interval in milliseconds
    html += "var updateTimer;";
    html += "var isUpdating = true;";
    html += "var startTime = new Date(new Date().getTime() + 2 * 60 * 60 * 1000).toISOString().slice(0, 16);";
    html += "var endTime = new Date(new Date().getTime() + 3 * 60 * 60 * 1000 * 24).toISOString().slice(0, 16);";
    html += "function fetchData() {";
    html += "  var xhr = new XMLHttpRequest();";
    html += "  xhr.onreadystatechange = function() {";
    html += "    if (xhr.readyState == 4 && xhr.status == 200) {";
    html += "      var data = JSON.parse(xhr.responseText);";
    html += "      console.log(data);"; // Log the fetched data
    html += "      updateChart(data);";
    html += "      updateDataList(data);";
    html += "    }";
    html += "  };";
    html += "  xhr.open('GET', '/data', true);";
    html += "  xhr.send();";
    html += "}";
    html += "function updateChart(data) {";
    html += "  var now = new Date();";
    html += "  chart.data.labels.push(now);";
    html += "  chart.data.datasets[0].data.push({x: now, y: data.temperature});";
    html += "  chart.data.datasets[1].data.push({x: now, y: data.humidity});";
    html += "  chart.data.datasets[2].data.push({x: now, y: data.co2});";
    html += "  chart.data.datasets[3].data.push({x: now, y: data.tvoc});";
    html += "  chart.data.datasets[4].data.push({x: now, y: data.lightLevel});";
    html += "  chart.data.datasets[5].data.push({x: now, y: data.rawH2});";
    html += "  chart.data.datasets[6].data.push({x: now, y: data.rawEthanol});";
    html += "  var start = new Date(document.getElementById('startTime').value);";
    html += "  var end = new Date(document.getElementById('endTime').value);";
    html += "  while (chart.data.labels.length > 0 && chart.data.labels[0] < start) {";
    html += "    chart.data.labels.shift();";
    html += "    chart.data.datasets.forEach(dataset => dataset.data.shift());";
    html += "  }";
    html += "  while (chart.data.labels.length > 0 && chart.data.labels[chart.data.labels.length - 1] > end) {";
    html += "    chart.data.labels.pop();";
    html += "    chart.data.datasets.forEach(dataset => dataset.data.pop());";
    html += "  }";
    html += "  chart.update();";
    html += "}";
    html += "function updateDataList(data) {";
    html += "  var dataList = document.getElementById('dataList');";
    html += "  dataList.innerHTML = 'Temperature: ' + data.temperature + ' C<br>' +";
    html += "                      'Humidity: ' + data.humidity + ' %<br>' +";
    html += "                      'CO2: ' + data.co2 + ' ppm<br>' +";
    html += "                      'TVOC: ' + data.tvoc + ' ppb<br>' +";
    html += "                      'Light: ' + data.lightLevel + ' lx<br>' +";
    html += "                      'Raw H2: ' + data.rawH2 + '<br>' +";
    html += "                      'Raw Ethanol: ' + data.rawEthanol + '<br>';";
    html += "}";
    html += "function toggleUpdate() {";
    html += "  isUpdating = !isUpdating;";
    html += "  var button = document.getElementById('toggleButton');";
    html += "  if (isUpdating) {";
    html += "    button.innerHTML = 'Stop Update';";
    html += "    startUpdate();";
    html += "  } else {";
    html += "    button.innerHTML = 'Start Update';";
    html += "    clearInterval(updateTimer);";
    html += "  }";
    html += "}";
    html += "function startUpdate() {";
    html += "  updateTimer = setInterval(fetchData, updateInterval);";
    html += "}";
    html += "function setUpdateInterval() {";
    html += "  var interval = document.getElementById('updateInterval').value;";
    html += "  updateInterval = parseInt(interval);";
    html += "  if (isUpdating) {";
    html += "    clearInterval(updateTimer);";
    html += "    startUpdate();";
    html += "  }";
    html += "}";
    html += "function setTimeRange() {";
    html += "  startTime = document.getElementById('startTime').value;";
    html += "  endTime = document.getElementById('endTime').value;";
    html += "  chart.update();";
    html += "}";
    html += "var ctx;";
    html += "var chart;";
    html += "window.onload = function() {";
    html += "  ctx = document.getElementById('sensorChart').getContext('2d');";
    html += "  chart = new Chart(ctx, {";
    html += "    type: 'line',";
    html += "    data: {";
    html += "      labels: [],";
    html += "      datasets: [";
    html += "        { label: 'Temperature', borderColor: 'red', fill: false, data: [], yAxisID: 'y' },";
    html += "        { label: 'Humidity', borderColor: 'blue', fill: false, data: [], yAxisID: 'y' },";
    html += "        { label: 'CO2', borderColor: 'green', fill: false, data: [], yAxisID: 'y1' },";
    html += "        { label: 'TVOC', borderColor: 'purple', fill: false, data: [], yAxisID: 'y1' },";
    html += "        { label: 'Light', borderColor: 'orange', fill: false, data: [], yAxisID: 'y' },";
    html += "        { label: 'Raw H2', borderColor: 'brown', fill: false, data: [], yAxisID: 'y1' },";
    html += "        { label: 'Raw Ethanol', borderColor: 'pink', fill: false, data: [], yAxisID: 'y1' }";
    html += "      ]";
    html += "    },";
    html += "    options: {";
    html += "      scales: {";
    html += "        x: {";
    html += "          type: 'time',";
    html += "          time: {";
    html += "            unit: 'minute',";
    html += "            tooltipFormat: 'MMM dd, yyyy, HH:mm:ss',";
    html += "            displayFormats: {";
    html += "              minute: 'HH:mm'";
    html += "            }";
    html += "          },";
    html += "          ticks: {";
    html += "            source: 'auto',";
    html += "            autoSkip: true";
    html += "          }";
    html += "        },";
    html += "        y: {";
    html += "          beginAtZero: true,";
    html += "          position: 'left'";
    html += "        },";
    html += "        y1: {";
    html += "          beginAtZero: true,";
    html += "          position: 'right',";
    html += "          grid: {";
    html += "            drawOnChartArea: false"; // only want the grid lines for one axis to show up
    html += "          }";
    html += "        }";
    html += "      }";
    html += "    }";
    html += "  });";
    html += "  document.getElementById('startTime').value = new Date(new Date().getTime() + 2 * 60 * 60 * 1000).toISOString().slice(0, 16);";
    html += "  document.getElementById('endTime').value = new Date(new Date().getTime() + 3 * 60 * 60 * 1000 * 24).toISOString().slice(0, 16);";
    html += "  startUpdate();"; // Start the update process
    html += "}";
    html += "</script></head><body>";
    html += "<h1>Sensor Data of " + instance->deviceName + "</h1>";
    html += "<div id='dataList'>Loading...</div>";
    html += "<div>";
    html += "  Update Interval (ms): <input type='number' id='updateInterval' value='1000' onchange='setUpdateInterval()'><br>";
    html += "  Start Time: <input type='datetime-local' id='startTime' onchange='setTimeRange()'><br>";
    html += "  End Time: <input type='datetime-local' id='endTime' onchange='setTimeRange()'><br>";
    html += "  <button id='toggleButton' onclick='toggleUpdate()'>Stop Update</button>";
    html += "</div>";
    html += "<canvas id='sensorChart' width='400' height='200'></canvas>";
    html += "</body></html>";
    instance->webServer.send(200, "text/html", html);
}

void PublishManager::handleData()
{
    String message = "{";
    message += "\"temperature\": " + String(instance->lastSensorData.temperature) + ",";
    message += "\"humidity\": " + String(instance->lastSensorData.humidity) + ",";
    message += "\"co2\": " + String(instance->lastSensorData.co2) + ",";
    message += "\"tvoc\": " + String(instance->lastSensorData.tvoc) + ",";
    message += "\"lightLevel\": " + String(instance->lastSensorData.lightLevel) + ",";
    message += "\"rawH2\": " + String(instance->lastSensorData.rawH2) + ",";
    message += "\"rawEthanol\": " + String(instance->lastSensorData.rawEthanol);
    message += "}";
    instance->webServer.send(200, "application/json", message);
}

// Callback function to renew IP address
void PublishManager::renewIpAddress(const WiFiEventStationModeGotIP &event)
{
    Serial.print("IP address was changed, IP: ");
    instance->ipAddress = event.ip;
    Serial.println(instance->ipAddress.toString());

    // Publish the new IP address
    if (instance->client.connected())
    {
        instance->client.publish((instance->deviceName + "/ipAddress").c_str(), instance->ipAddress.toString().c_str());
    }
}

// Method to setup mqtt
void PublishManager::setupMqtt()
{
    client.setClient(espClient);
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(mqttCallback);
}

// Method to reconnect to the mqtt broker
void PublishManager::reconnectMqtt()
{
    // Loop until we're reconnected
    while (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (client.connect(deviceName.c_str(), mqtt_user, mqtt_password))
        {
            Serial.println("connected");
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

// Callback function to handle incoming mqtt messages
void PublishManager::mqttCallback(char *topic, byte *payload, unsigned int length)
{
    String message;

    Serial.println("Message arrived [" + String(topic) + "] ");

    for (unsigned int i = 0; i < length; i++)
    {
        message += (char)payload[i];
    }

    if (String(topic) == (instance->deviceName + "/dataUpdateTime"))
    {
        // Update dataUpdateTime
        int newValue = message.toInt();
        if (newValue > 0)
        {
            Serial.print("Updating dataUpdateTime to ");
            Serial.println(newValue);
        }
    }
}

// Method to publish sensor values on Serial
void PublishManager::publishOnSerial(SensorData sensorData)
{
    lastSensorData = sensorData;
    // Print sensor values to the serial
    Serial.print("Temperature: ");
    Serial.print(sensorData.temperature, 2); // Print temperature with 2 decimal places
    Serial.print(" C, Humidity: ");
    Serial.print(sensorData.humidity, 2); // Print humidity with 2 decimal places
    Serial.print(" %, TVOC: ");
    Serial.print(sensorData.tvoc);
    Serial.print(" ppb, CO2: ");
    Serial.print(sensorData.co2);
    Serial.print(" ppm, Raw ethanol: ");
    Serial.print(sensorData.rawEthanol);
    Serial.print(", Raw H2: ");
    Serial.print(sensorData.rawH2);
    Serial.println();
}

// Method to publish sensor values on mqtt
void PublishManager::publishOnMqtt(SensorData sensorData)
{
    lastSensorData = sensorData;
    // Connect to the mqtt broker
    if (!client.connected())
    {
        reconnectMqtt();
    }
    client.loop();

    // Publish sensor values
    if (client.publish((deviceName + "/temperature").c_str(), String(sensorData.temperature).c_str()) == false)
    {
        Serial.println("Temperature not published");
    }
    if (client.publish((deviceName + "/humidity").c_str(), String(sensorData.humidity).c_str()) == false)
    {
        Serial.println("Humidity not published");
    }
    if (client.publish((deviceName + "/tvoc").c_str(), String(sensorData.tvoc).c_str()) == false)
    {
        Serial.println("TVOC not published");
    }
    if (client.publish((deviceName + "/co2").c_str(), String(sensorData.co2).c_str()) == false)
    {
        Serial.println("CO2 not published");
    }
    if (client.publish((deviceName + "/rawEthanol").c_str(), String(sensorData.rawEthanol).c_str()) == false)
    {
        Serial.println("Raw ethanol not published");
    }
    if (client.publish((deviceName + "/rawH2").c_str(), String(sensorData.rawH2).c_str()) == false)
    {
        Serial.println("Raw H2 not published");
    }

    // Publish ip address
    if (client.publish((deviceName + "/ipAddress").c_str(), ipAddress.toString().c_str()) == false)
    {
        Serial.println("IP address not published");
    }
}