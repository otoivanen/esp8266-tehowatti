#ifndef INDEX_HTML_H
#define INDEX_HTML_H

const char* index_html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Jäspi TehoWatti Online</title>
  <style>
    body {
      font-family: 'Arial', sans-serif;
      background-color: #f0f4f8;
      color: #333;
      margin: 0;
      padding: 0;
      display: flex;
      justify-content: center;
      align-items: center;
      height: 100vh;
    }

    .container {
      background-color: #ffffff;
      border-radius: 10px;
      box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
      padding: 20px 40px;
      max-width: 500px;
      width: 90%;
      text-align: center;
    }

    h1 {
      font-size: 2.5em;
      color: #4CAF50;
      margin-bottom: 20px;
    }

    form {
      display: flex;
      flex-direction: column;
      gap: 15px;
    }

    label {
      font-size: 1em;
      font-weight: bold;
      text-align: left;
      color: #555;
      margin-bottom: 5px;
    }

    input {
      width: 100%;
      padding: 10px;
      border: 1px solid #ccc;
      border-radius: 5px;
      box-sizing: border-box;
      font-size: 1em;
    }

    button {
      background-color: #4CAF50;
      color: white;
      padding: 10px 20px;
      font-size: 1.2em;
      border: none;
      border-radius: 5px;
      cursor: pointer;
      transition: background-color 0.3s;
    }

    button:hover {
      background-color: #45a049;
    }

    .footer {
      margin-top: 20px;
      font-size: 0.9em;
      color: #777;
    }

    @media (max-width: 600px) {
      .container {
        padding: 15px 20px;
      }

      h1 {
        font-size: 2em;
      }
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>Jäspi TehoWatti Online</h1>
    <form id="settingsForm">
      <label for="ssid">WiFi SSID</label>
      <input type="text" id="ssid" name="ssid" placeholder="Enter WiFi SSID" required>

      <label for="password">WiFi Password</label>
      <input type="password" id="password" name="password" placeholder="Enter WiFi Password" required>

      <label for="mqttBroker">MQTT Broker Address</label>
      <input type="text" id="mqttBroker" name="mqttBroker" placeholder="e.g., 192.168.1.100" required>

      <label for="mqttPort">MQTT Broker Port</label>
      <input type="number" id="mqttPort" name="mqttPort" placeholder="e.g., 1883" required>

      <label for="mqttTopic1">MQTT Topic for Sensor 1</label>
      <input type="text" id="mqttTopic1" name="mqttTopic1" placeholder="e.g., sensor1/temp" required>

      <label for="mqttTopic2">MQTT Topic for Sensor 2</label>
      <input type="text" id="mqttTopic2" name="mqttTopic2" placeholder="e.g., sensor2/temp" required>

      <label for="mqttRelay">MQTT Topic for Relay Control</label>
      <input type="text" id="mqttRelay" name="mqttRelay" placeholder="e.g., relay/control" required>

      <button type="button" id="saveButton">Save Settings</button>
    </form>
    <div class="footer">© 2024 Jäspi TehoWatti Online</div>
  </div>

  <script>
    // Save Settings button event listener
    document.getElementById('saveButton').addEventListener('click', async function(event) {
      event.preventDefault(); // Prevent form submission

      // Generate url for the http request
      const saveSettingsEndpoint = "http://192.168.1.177"//window.location.host;
      const endpoint = "savesettings";
      const separator = saveSettingsEndpoint.endsWith('/') ? '' : '/';
      const fullUrl = saveSettingsEndpoint + separator + endpoint;

      console.log(fullUrl);

      // Generate form object
      const form = document.getElementById('settingsForm');
      const formData = new FormData(form);
      const settings = Object.fromEntries(formData.entries());
      console.log('Settings:', settings);

      try {
        const response = await fetch(fullUrl, {
          method: "POST",
          headers: {
            'Content-Type': 'application/json'
          },
          body: JSON.stringify(settings)
        });

        if (!response.ok) {
          console.log("Http request failed");
        } else {
          console.log(await response.json());
        }
      } catch (error) {
        console.error('Error:', error);
      }
    });
  </script>
</body>
</html>
)rawliteral";

#endif
