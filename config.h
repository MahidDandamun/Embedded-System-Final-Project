#ifndef CONFIG_H
#define CONFIG_H

// Include auto-generated secrets (run generate_secrets.py first)
#ifdef USE_SECRETS
#include "secrets.h"
#else
// Placeholder values - replace with your secrets system
#define WIFI_SSID "PLACEHOLDER_SSID"
#define WIFI_PASSWORD "PLACEHOLDER_PASSWORD"
#define API_KEY "PLACEHOLDER_API_KEY"
#define API_SECRET "PLACEHOLDER_SECRET"
#endif

#endif