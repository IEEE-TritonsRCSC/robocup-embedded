#line 1 "C:\\Users\\bjsek\\Documents\\GitHub\\robocup-embedded\\robocup\\robocup\\README.md"
# UCSD Triton Bots Embedded Code
2025-2026 UCSD Triton Bots RoboCup embedded code for the Arduino Uno Q board
# Notes
The main code is contained in the robocup.ino file
This Arduino sketch uses the Arduino Maker Workshop extension in VSCode
The build directory, arduino.json, c_cpp_properties.json, and sketch.yaml file contain the settings for the build profile to make the code portable and stable

# Instructions
## Install Tailscale (do once)
`curl -fsSL https://tailscale.com/install.sh | sh # install tailscale`

`sudo tailscale up # start tailscale connection`

The sudo password is 12345678

`tailscale status # check connection`
## Tailscale Console
Open the tailscale console to find the Arduino UNO Q's IP address
Copy the IP Address and set `ARDUINO_IP` to the IP in the `send.py` script
## Configure WiFi
The wifi port numbers can be anything but choose something between 4000 and 20000
# Getting Started
- Make sure the Arduino Maker Workshop VS Code Extension is installed
- Open the sketch folder `robocup`
- Go to sketch.yaml and change port: COMX your USB COM port. You can check "device manager" on windows to find the COM number
- Upload code in the Arduino Workshop left panel
- Unplug board, wait 10 seconds, replug board
- SSH into the UNO Q, which can be done by opening the Arduino App Lab and clicking the terminal button in the bottom left
- run `sudo tailscale up`
- Check the tailscale console to confirm the device is online
- Wait 30 seconds for the Linux Bridge to boot up
- Check Serial Monitor for setup confirmation message
- Run `send.py` and check Serial Monitor for receive confirmation from the UNO Q
 