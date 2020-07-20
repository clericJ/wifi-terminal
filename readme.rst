=================================================================================================
Wireless Terminal - simple, configurable with web, wifi to uart bridge, using esp8266
=================================================================================================

Code writter using Visual Studio Code and PlatformIO plugin.
At startup, the controller creates a WirelessTerminal access point with the password 123456789.
Configurable with web interface (192.168.4.1 address by default).

Прошивка написана в Visual Studio Code с использованием PlatformIO
Настройка осуществляется с помощью web интерфейса.
При старте, контроллер создаёт точку доступа WirelessTerminal с паролем 123456789.
Для настройки следует открыть в браузере адрес 192.168.4.1 (по умолчанию).
Для соединения с устройством через uart (или rs232 через преобразователь).
следует открыть TCP терминал (telnet клиент) и подключится к контроллеру по порту 23
например:
telnet 192.168.4.1

Screenshots
-----------

Configuration:
.. figure:: https://raw.githubusercontent.com/clericJ/wifi-terminal/master/img/web-interface.jpg

Web Terminal:
.. figure:: https://raw.githubusercontent.com/clericJ/wifi-terminal/master/img/web-terminal.jpg

Wifi Serial Android App
.. figure:: https://raw.githubusercontent.com/clericJ/wifi-terminal/master/img/wifi-serial-android-app.jpg

Wemos D1 Mini rs232 shield
==========================
schematic: https://easyeda.com/clericJ/wemos-d1mini-rs232shield
-----------------
.. figure:: https://raw.githubusercontent.com/clericJ/wifi-terminal/master/img/esp-and-shield.jpg
.. figure:: https://raw.githubusercontent.com/clericJ/wifi-terminal/master/img/rs232-shield.jpg
