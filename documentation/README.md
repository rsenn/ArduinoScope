User guide
===
This application allows to read, display and export multiple data coming from an Arduino device in real time.

Launch the application
---

Connect the Arduino to the computer, upload the Arduino program that corresponds to your hardware setup and make sure you are receiving data in the Arduino IDE terminal.

Close the Arduino IDE and launch the application:

![Arduino](https://github.com/VictorLamoine/arduinoscope/raw/master/documentation/Arduino.png)

If the list of ports is empty, the Arduino is either already busy or not properly connected to the computer. Tweak the baudrate to match your setup.

![MainWindow](https://github.com/VictorLamoine/arduinoscope/raw/master/documentation/MainWindow.png)

You can resize the window and the right panel by dragging it left/right.

Toolbar
---
![media-record](https://github.com/VictorLamoine/arduinoscope/raw/master/icons/media-record.png)
- Start recording data, click again to stop recording.
- By default a text file (CSV) containing the measurements is written on the disk in the default folder at the end of the recording. The name of the file is composed of the date and time at the end of the recording. The values are `;` separated. You can change the decimal separator in the application parameters.

If you disconnect the Arduino from the computer during the recording the application won't crash but you will have to start it again to get the Arduino connexion working again.

![camera-photo](https://github.com/VictorLamoine/arduinoscope/raw/master/icons/camera-photo.png)
Take a screen-shot of the current graph.

![preferences-system.png](https://github.com/VictorLamoine/arduinoscope/raw/master/icons/preferences-system.png)
Allows you to change the application parameters.

![Parameters](https://github.com/VictorLamoine/arduinoscope/raw/master/documentation/Parameters.png)

- `Auto write CSV files`: if ticked the application will write a CSV file at the end of the recording in the default directory.
- `Decimal separator`: choose the decimal separator; `20.25` (english) or `20,25` (french) for the CSV files.
- `Number of graphs`: Allows to choose the number of data to gather from the Arduino. This must match the number of data sent by the Arduino.

![GraphsProperties](https://github.com/VictorLamoine/arduinoscope/raw/master/documentation/GraphsProperties.png)

You can tweak each graph label, unit and display or not the ticks. Two labels cannot be identical, the label cannot be empty.

![document-save.png](https://github.com/VictorLamoine/arduinoscope/raw/master/icons/document-save.png)
Choose the default directory where the recording files will be written. Only applies if `Auto write CSV files` is true.

![help.png](https://github.com/VictorLamoine/arduinoscope/raw/master/icons/help.png)
Shows software version and miscellaneous information.
