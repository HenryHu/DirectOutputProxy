# DirectOutputProxy

This simple app serves as a proxy for Saitek/Logitech DirectOutput devices.

## Devices

Supported devices:

* Logitech X52 Pro

Incomplete support:

* Saitek Flight Instrument Panel - I don't have this device.

## Usage

Run the app. It'll run in the background.

Access its services through its web API. By default it listens on port 8080. Port can be specified on the command line.

The default page shows the current status, including the devices recognized and their configuration.

Other apps can call the API to control the devices.

## API

The app provides a simple HTTP-based API. Available methods:

* `/setline/<page index>/<line index>?content=<line content>`

  Sets the content of a line on the X52 Pro LCD.

  The page index starts from 0. 2 pages are added by default. Pages can be added/removed using other methods.

  Line index can be 0/1/2, corresponding to the top/middle/bottom lines on the LCD.

* `/addpage/<page index>/<activate>[?name=<page name>][&top=<top line content>][&middle=<middle line content>][&bottom=<bottom line content>]`

  Adds a new page, and optionally make it the current page.

  By default the page is empty. The content of the lines can be specified as well.
  
* `/delpage/<page index>`

  Deletes a page.

* `/exit`

  Terminates the app.

## Runtime Dependency

The X52 Pro driver should be installed first. This app depends on the DirectOutput library it installs.

This installation should install the DirectOutput library into `C:\Program Files\Logitech\DirectOutput`.
In addition, it should set the library path in the Registry, at `HKEY_LOCAL_MACHINE\SOFTWARE\Saitek\DirectOutput`.
Notice that it sets the path in the `DirectOutput_Saitek` key. The example in the SDK uses the `DirectOutput` key, so if the DLL is not found, check if the key is right.

## Build Dependency

This project has a few dependencies:
* DirectOutput SDK, installed into  `C:\Program Files\Logitech\DirectOutput\SDK` by default.
* Crow (https://github.com/CrowCpp/Crow)
* Asio (Crow depends on it, https://think-async.com/Asio/)

Their include directories should be added to 'External include directories'.

Also, check the documentations in the DirectOutput SDK directory.
