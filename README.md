# Joy-Con Shutdown Tool (macOS)

This tool allows you to completely shut down Nintendo Switch controllers (Joy-Cons, Pro Controller, etc.) to a factory state. This is useful for storing controllers for long periods without battery drain.

## ⚠️ WARNING

This program writes to the SPI flash of your Nintendo Switch controller. While the operations performed are safe when used correctly, any mistakes in the process could potentially brick your controller. **Use at your own risk!**

## Features

- Puts the controller into low power mode (factory state)
- Removes pairing information
- Works with:
  - Joy-Con (L)
  - Joy-Con (R)
  - Pro Controller
  - SNES Controller
  - N64 Controller
  *after porting it to macOS, I only tested it myself with Joy-Cons and a Pro Controller, but it should work with other listed controllers as well*

## Prerequisites

You need to have the following installed:

1. Xcode Command Line Tools
2. CMake
3. HIDAPI library

To install the prerequisites using Homebrew:

```bash
# Install Homebrew if you haven't already
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install the required packages
brew install cmake hidapi
```

## Building

1. Clone this repository:
```bash
git clone https://github.com/yourusername/joycon-turnoff.git
cd joycon-turnoff
```

2. Create a build directory and build the project:
```bash
mkdir build
cd build
cmake ..
make
```

## Usage

1. Make sure your controller is connected to your Mac via Bluetooth
   - To pair a new controller, press and hold the sync button (small round button) until the LEDs start flashing
   - Use macOS Bluetooth settings to complete the pairing

2. Run the program:
```bash
./jcoff
```

3. Follow the on-screen instructions carefully

## After Shutdown

To wake up the controller after shutdown:
- Connect it to the Switch with a USB cable, or
- Perform Bluetooth pairing again by holding the sync button

## Troubleshooting

1. **Controller not detected**
   - Make sure the controller is connected via Bluetooth
   - Try disconnecting and reconnecting the controller
   - Ensure you have the necessary permissions to access HID devices

2. **Build fails**
   - Make sure all prerequisites are installed
   - Try running `brew update && brew upgrade` to update all packages
   - Check if HIDAPI is properly installed with `brew info hidapi`

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Credits

This is a macOS port of the original Windows tool. The core functionality and protocol implementation remain the same.

For the more info about the original tool, see: https://github.com/Sopsy/joycon-turnoff
