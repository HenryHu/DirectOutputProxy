# DirectOutputProxy

This simple app serves as a proxy for Saitek/Logitech DirectOutput devices.

## Devices

Supported devices:

* Logitech X52 Pro

Incomplete support:

* Saitek Flight Instrument Panel - I don't have this device.

## Usage

Run the app. It'll run in the background.
Access its services through its web API. By default it listens on port 8080.
The default page shows the current status, including the devices recognized and their configuration.
Other apps can call the API to control the devices.

## API

The app provides a simple HTTP-based API. Available methods:

* /setline/<page index>/<line index>?content=<line content>

  Sets the content of a line on the X52 Pro LCD.
  The page index starts from 0. 2 pages are added by default. Pages can be added/removed using other methods.
  Line index can be 0/1/2, corresponding to the top/middle/bottom lines on the LCD.

* /addpage/<page index>/<activate>[?name=<page name>][&top=<top line content>][&middle=<middle line content>][&bottom=<bottom line content>]

  Adds a new page, and optionally make it the current page.
  By default the page is empty. The content of the lines can be specified as well.
  
* /delpage/<page index>

  Deletes a page.

* /exit

  Terminates the app.
