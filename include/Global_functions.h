/*
Headers for declaring global functions accessible by all classes. Global functions are used in the main program when sequence of actions
need to be performed for multiple classes repeatedly and called from various classes, e.g. setting relay state and updating LED & OLED
via MQTT-messagehandler in main program or a HTTP-route from WebServerManager class.

Date: 2025-02-25
Author: Oskari Toivanen
*/
#ifndef GLOBAL_FUNCTIONS_H
#define GLOBAL_FUNCTIONS_H

void setRelayState(bool state);
String getStatesAsJson();

#endif