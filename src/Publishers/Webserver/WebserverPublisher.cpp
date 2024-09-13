// PublishManager.cpp

#include "WebserverPublisher.h"
#include <Arduino.h>
#include "config.h"
#include "ESP8266WebServer.h"
#include <ESP8266mDNS.h>

// Initialize the static instance pointer
WebserverPublisher *WebserverPublisher::instance = nullptr;

// Constructor to initialize WebserverPublisher
WebserverPublisher::WebserverPublisher() : webServer(80)
{
    instance = this;
}

void WebserverPublisher::setup(String deviceName)
{
    instance->deviceName = deviceName;

    // Initialize web server
    instance->webServer.on("/", handleRoot);
    instance->webServer.on("/data", handleData);
    instance->webServer.begin();
    MDNS.begin("esp8266");
    Serial.println("HTTP server started");
}

// Implement interface method publish
void WebserverPublisher::publish(const SensorData &data)
{
    Serial.println("WebserverPublisher: Publishing sensor data");

    instance->sensorData = data;
    // Handle the web server requests
    instance->webServer.handleClient();
    MDNS.update();
}

void WebserverPublisher::handleRoot()
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

void WebserverPublisher::handleData()
{
    String message = "{";
    message += "\"temperature\": " + String(instance->sensorData.temperature) + ",";
    message += "\"humidity\": " + String(instance->sensorData.humidity) + ",";
    message += "\"co2\": " + String(instance->sensorData.co2) + ",";
    message += "\"tvoc\": " + String(instance->sensorData.tvoc) + ",";
    message += "\"lightLevel\": " + String(instance->sensorData.lightLevel) + ",";
    message += "\"rawH2\": " + String(instance->sensorData.rawH2) + ",";
    message += "\"rawEthanol\": " + String(instance->sensorData.rawEthanol);
    message += "}";
    instance->webServer.send(200, "application/json", message);
}
