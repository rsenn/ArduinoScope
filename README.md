<img src="https://github.com/VictorLamoine/arduinoscope/raw/master/icons/application-icon.png" align="center" height="100"> ArduinoScope
===
Record, display and export multiple sensors data coming from your Arduino.

Travis CI
---
[![Travis-CI](https://travis-ci.org/VictorLamoine/ArduinoScope.svg?branch=master)](https://travis-ci.org/VictorLamoine/ArduinoScope/branches) 

![MainWindow](https://github.com/VictorLamoine/arduinoscope/raw/master/documentation/MainWindow.png)

Dependencies
---
- [Qt 5.5](http://www.qt.io/download-open-source/) (with `QPrintSupport` and `QSerialPort` support)

Install Qt on Ubuntu:
```bash
sudo apt-get install qttools5-dev qt5-default qtmultimedia5-dev qtmultimedia5-examples libqt5serialport5-dev
```

How to build
---
Clone the project or [download](https://github.com/VictorLamoine/ArduinoScope/archive/master.zip) the latest archive.

Open the `.pro` file and build/launch the application (`Ctrl+R`).

Or build manually using `qmake`:
```bash
mkdir build
cd build
qmake ../arduinoscope.pro
make -j2
```

Or build manually using `cmake`:
```bash
mkdir build
cd build
cmake ..
make -j2
```

Documentation
---
[User guide](documentation/README.md)

Arduino code
---
The [Arduino](documentation/arduino) directory contains example Arduino codes to be able to use the application.

Licence
---
See [LICENSE.txt](LICENSE.txt). This project uses `QCustomPlot`, please read [the library licence](qcustomplot/GPL.txt).
