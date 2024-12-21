document.addEventListener('DOMContentLoaded', function(){ 
// Save Settings button event listener
  document.getElementById('saveButton').addEventListener('click', async function(event) {
   event.preventDefault(); // Prevent form submission

  // Generate url for the http request
  //const saveSettingsEndpoint = "http://192.168.1.177"
  const host = window.location.host;
  const endpoint = "savesettings";
  const http = host.startsWith('http://') ? '' : 'http://';
  const separator = host.endsWith('/') ? '' : '/';
  const fullUrl = http + host + separator + endpoint;

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
      console.log("Http request failed:", response.status);
    }
        
    const textResponse = await response.text();
    alert(textResponse);

  } catch (error) {
    console.error('Error:', error);
  }
});
});