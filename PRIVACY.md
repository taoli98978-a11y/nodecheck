# Privacy policy

NodeCheck is a local Windows application. It has no app account, analytics, cloud credential storage or background auto-update service.

When the user clicks the check button:
- The proxy hostname is resolved locally and the app connects to that proxy gateway. Proxy authentication credentials are sent to the chosen proxy; SOCKS5 username/password authentication is not itself encrypted. Use a trusted network/provider.
- SOCKS5/SOCKS5H target hostnames are sent to the proxy for remote resolution. HTTPS exit-IP requests use ipify, with icanhazip fallback.
- Direct HTTPS requests query ipwho.is and IP2Location.io using the detected exit IP. These services can see the querying connection's public IP and the submitted proxy exit IP.
- Region lookups may query public postal/telephone websites, OpenStreetMap Nominatim/Overpass, Google and DuckDuckGo. They receive the location-related query and the querying connection's public IP. Their availability and privacy policies apply.

Proxy credentials stay in application memory; there is no intentional disk persistence. Copy buttons containing the node also copy its password to the Windows clipboard; clipboard history/sync and other applications may access it according to Windows settings.
Closing the application ends its own processing. The application does not clear Windows clipboard history.

The bundled postal/housing reference data is public regional data, not a record of the user's location. Lookup results cannot establish a device's actual residence.

Source-only development tools can cache public test responses in test-cache. Those caches must not be published. No real user proxy credentials are included in the public fixtures.

Provider information: https://www.ipify.org/ ; https://icanhazip.com/ ; https://ipwho.is/ ; https://www.ip2location.io/ ; https://www.openstreetmap.org/ ; https://www.google.com/ ; https://duckduckgo.com/ . See the actual endpoint list in lookup.h for supplemental lookup websites.
