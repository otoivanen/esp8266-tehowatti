/*
On load, generate the baseurl from browser's current IP because page might be served from unknown IP
*/
const host = window.location.host;
const http = host.startsWith('http://') ? '' : 'http://';
const separator = host.endsWith('/') ? '' : '/';
const fullUrl = http + host + separator;
console.log(fullUrl);

/*
Declare global variables to hold configs & states received from device accessible for all functions
*/
let configJson = {};
let statesJson = {};
let statusInterval = null; // Placeholder for interval to refresh states in background

/*
When the DOM is first loaded, fetch the full config from device through settings -endpoint,
load the statuspage -fragment and populate it with current device status
*/
document.addEventListener("DOMContentLoaded", async () => {
  configJson = await loadData(fullUrl+'settings');
  
  await loadPage(fullUrl+'status');
  await populateStates();
});

/*
Attach event listener to wifiConfigMenu button with command to load the WiFiConfig -fragment
and pre-populate the form from existing config (except password) if available.

Following functions work in the same manner, load page and populate data
*/
document.getElementById("wifiConfigMenu").addEventListener('click', async () => {
  await loadPage(fullUrl+'wificonfig');
  await populateWifiForm();
});

// Load MQTT config page
document.getElementById("settingsMenu").addEventListener('click', async () => {
  await loadPage(fullUrl+'mqttconfig');
  await populateMqttForm();
});

// Load status page
document.getElementById("statusMenu").addEventListener('click', async () => {
  await loadPage(fullUrl+'status');
  await populateStates();
})

// Load sensor config page
document.getElementById("sensorMenu").addEventListener('click', async () => {
  await loadPage(fullUrl+'sensorconfig');
  document.getElementById('getSensorsButton').addEventListener('click', getAvailableSensors);
})

/*
A generic function to load HTML fragments into mainContainer. Takes full url as parameter
and fetches the fragment from device's endpoint serving the fragment.

If the loaded fragment was status-section, 5sec reload interval is initiated that re-populates
the temperatures every 5secs. If any other fragment was loaded, the interval is disabled.
*/
async function loadPage(url) {
  try {
    const response = await fetch(url);
    if (!response.ok) throw new Error('Network response was not ok');

    const html = await response.text();
    document.getElementById('mainContainer').innerHTML = html;

    if (url.includes("status")) {
      startStatusInterval();
    } else {
      stopStatusInterval();
    }

  } catch (error) {
    console.error(`Error fetching ${url}:`, error);
  }

}

/*
A generic function to fetch JSON-data, e.g. statuses, configs and sensordata for updating
the UI elements dynamically. Takes the endpoint url as parameter and returns JSON content.
*/
async function loadData(url) {
  try {
    const response = await fetch(url);
    if (!response.ok) throw new Error('Network response was not ok');

    const data = await response.json();
    console.log(data);
    return data;

  } catch (error) {
    console.error(`Error fetching ${url}:`, error);
  }

};

/*
Add event delegation listeners for components that are dynamically loaded into DOM and not necessarily visible on page load
Define url & status mappings for certain id's that are utilized when elements appear in DOM.
*/
document.addEventListener('click', async (event) => {
  // Map the button actions and statustext based on their id
  const buttonActions = {
    on: { url: fullUrl+"relay?state=ON", status: "Active"},
    off: { url: fullUrl+"relay?state=OFF", status: "Inactive" },
    restart: { url: fullUrl+"restart" } // Different URL for restart
  };

  // Attach functionality to buttons defined in buttonActions mapping based on id's received from event
  if (event.target && buttonActions.hasOwnProperty(event.target.id)) {
    try {
      // Assign the url to fetch from the mapping
      const response = await fetch(buttonActions[event.target.id].url);
      const text = await response.text();

      // Define the detailed actions based on button id
      if (response.ok) {
        if (event.target.id == "on") {
          document.getElementById("relaystate").innerHTML = buttonActions[event.target.id].status;
        } else if (event.target.id == "off") {
          document.getElementById("relaystate").innerHTML = buttonActions[event.target.id].status;
        } else if (event.target.id == "restart") {
          alert(text);
        }
      }
    } catch (error) {
      console.error("Failed sending command", error);
    }
  }
});

/*
Event delegation listener to catch and process all forms in various HTML-fragments. OnClick 
action is attached to all buttons whose nearest element is form. Button's default submit action
is prevented and a JSON object is formed dynamically from the form where button resides.

The form contents are sent into endpoind handling config saving and validation as JSON-object with
POST request - server side (device) handles the form validation through configmanager setters and
returns OK if validation passed and failed if validation did not pass (e.g. values missing or
wrongly structured IP-address).

Error or prompt to restart device is being sent back to browser from server side.
 */
document.addEventListener('click', async function(event) {
  // Check if the clicked element is a button inside a form
  if (event.target.tagName === 'BUTTON' && event.target.closest('form')) {
    event.preventDefault();

    const form = event.target.closest('form'); // Get the parent form
    const formId = form.id; // Get the form ID
    
    // Collect form data
    const formData = new FormData(form);
    const settings = Object.fromEntries(formData.entries());
    console.log('Form Data:', settings);

    try {
      const response = await fetch(fullUrl+'settings', {
        method: "POST",
        headers: {
          'Content-Type': 'application/json'
        },
        body: JSON.stringify(settings)
      });

      if (!response.ok) {
        console.log("http request failed when saving configs:", response.status);
      }

      const textResponse = await response.text();
      alert(textResponse);

    } catch (error) {
      console.error('Error:', error);
    }

  }
});

// Populate the WiFi form when wifi fragment is being loaded
async function populateWifiForm() {

  // Set the SSID but not the password if exists
  "ssid" in configJson && (document.getElementById('ssid').value = configJson.ssid);
};

/*
Pre-populates the Mqtt config form if configs already exist, except for passwords. The configmanager defaults to (IP Unset) and port 0
which are handled here, to show the placeholder values if no config exist.

Rest of the configs defaults to empty strings which can be simply placed onto form if value exists.
*/
async function populateMqttForm() {
  // In case mqttPort and mqttServer defaults to (IP Unset) or 0 we don't want to populate them. Rest of the values default to "" which are not populated.
  if (configJson.mqttServer && configJson.mqttServer !== "(IP unset)") {
    document.getElementById("mqttServer").value = configJson.mqttServer
  };
  
  if (configJson.mqttPort > 0) {
    document.getElementById("mqttPort").value = configJson.mqttPort
  };

  "mqttUser" in configJson && (document.getElementById("mqttUser").value = configJson.mqttUser);
  "inletTempStateTopic" in configJson && (document.getElementById("inletTempStateTopic").value = configJson.inletTempStateTopic);
  "outletTempStateTopic" in configJson && (document.getElementById("outletTempStateTopic").value = configJson.outletTempStateTopic);
  "relayStateTopic" in configJson && (document.getElementById("relayStateTopic").value = configJson.relayStateTopic);
  "relaySetTopic" in configJson && (document.getElementById("relaySetTopic").value = configJson.relaySetTopic);


}

/*
Function fetches the known states from device through HTTP-request, and populates the status page with
fresh values. The returned WiFi -mode and mqttState are mapped to more meaningful texts.
*/
async function populateStates() {
  // Load all the states with http request to global variable
  statesJson = await loadData(fullUrl+'states');

  // Set wifi states
  const wifiStates = {
    "STA": "Station",
    "AP": "Access Point"
  };

  document.getElementById("wifimode").innerHTML = wifiStates[statesJson.wifiMode] || "N/A";
  document.getElementById("ssid").innerHTML = statesJson.SSID || "N/A";
  document.getElementById("wifiIP").innerHTML = statesJson.wifiIP || "N/A";

  const mqttStates = {
    true: "Connected",
    false: "Disconnected"
  };

  // Set mqtt states
  document.getElementById("mqttstatus").innerHTML = mqttStates[statesJson.MQTTConnected] || "N/A";
  document.getElementById("mqttIP").innerHTML = statesJson.MQTTBroker || "N/A";
  document.getElementById("mqttPort").innerHTML = statesJson.MQTTPort || "N/A";

  // Set sensor states
  document.getElementById("inlet").innerHTML = parseFloat(statesJson.InletTemp).toFixed(2) + "°C" || "N/A";
  document.getElementById("inletSensorAddress").innerHTML = statesJson.inletSensorAddress || "N/A"
  document.getElementById("outlet").innerHTML = parseFloat(statesJson.OutletTemp).toFixed(2) + "°C" || "N/A";
  document.getElementById("outletSensorAddress").innerHTML = statesJson.outletSensorAddress || "N/A"

  // Set relay states
  const relayStates = {
    "ON": "Active",
    "OFF": "Inactive"
  };

  document.getElementById("relaystate").innerHTML = relayStates[statesJson.RelayState] || "N/A";
}

/*
Function fetches the sensor address & temperature pairs from backend after pressing "Find available sensors" -button,
and let's user assign the inlet- and outlet temps through dropdown menu.

After scanning the sensordata from user's request, if backend sent 0 sensoraddresses error message is shown. Else 
a table of sensor addresses and current temperatures are generated and the table is added to DOM, and dropdown -menus
are populated with available sensor addresses. User can select which address is inlet- and outletsensor and save the configs.
*/
async function getAvailableSensors() {
  console.log("Fetchign available sensors");

  let sensorData = await loadData(fullUrl+"sensors")

  let sensorDiv = document.getElementById("availableSensors");
  let inletDropdown = document.getElementById("inletSensorAddress");
  let outletDropdown = document.getElementById("outletSensorAddress");


  // Clear the div
  sensorDiv.innerHTML = "<label>Available sensors:</label>";
  inletDropdown.innerHTML = '<option value="">Select a sensor</option>';
  outletDropdown.innerHTML = '<option value="">Select a sensor</option>';

  if (!sensorData.sensors || sensorData.sensors.length === 0) {
    sensorDiv.innerHTML += "<p>No sensors found, check the wiring ⚠️"
  } else {
    // Generate a table
    let table = document.createElement("table");
    table.classList.add("sensor-table");

    // Headers
    let thead = table.createTHead();
    let headerRow = thead.insertRow();
    ["Address", "Temperature (°C)"].forEach((text) => {
      let th = document.createElement("th");
      th.textContent = text;
      th.style.border = "1px solid black";
      th.style.padding = "5px";
      headerRow.appendChild(th);
    });

    // Create table body
    let tbody = table.createTBody();
    sensorData.sensors.forEach((sensor) => {
      // Add dropdown options
      let option1 = document.createElement("option");
      option1.value = sensor.address;
      option1.textContent = sensor.address;
      inletDropdown.appendChild(option1);

      let option2 = document.createElement("option");
      option2.value = sensor.address;
      option2.textContent = sensor.address;
      outletDropdown.appendChild(option2);

      // Add rows to table
      let row = tbody.insertRow();

      // Address column
      let addressCell = row.insertCell();
      addressCell.textContent = sensor.address;

      // Temperature column
      let tempCell = row.insertCell();
      tempCell.textContent = sensor.temperature.toFixed(1) + "°C"; // 1 decimal place
    });

    // Append table to div
    sensorDiv.appendChild(table);
  }
}

/*
Function starts the statusInterval that fetches sensor values for status-fragment every 5seconds when visible
by calling the populateStates() function, which dynamically updates the UI elements.
*/
function startStatusInterval() {
  if (!statusInterval) {
      statusInterval = setInterval(async () => {
        await populateStates();
      }, 5000); // Every 5 seconds
  }
}

// Function to stop the status update interval
function stopStatusInterval() {
  if (statusInterval) {
      clearInterval(statusInterval);
      statusInterval = null;
  }
}