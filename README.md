# Joy-Con Shutdown Tool

This tool allows you to completely shut down Nintendo Switch controllers (Joy-Cons, Pro Controller, etc.) to a factory state. This is useful for storing controllers for long periods without battery drain.

## ⚠️ WARNING

This program writes to the SPI flash of your Nintendo Switch controller. While the operations performed are safe when used correctly, any mistakes in the process could potentially brick your controller. **Use at your own risk!**

## Features

- Puts the controller into low power mode (factory state)
- Removes pairing information
- Works with:
  - Joy-Cons
  - Pro Controller
  - NES Joy-Cons
  - SNES Controller
  - N64 Controller

  **after porting it to macOS/SteamOS, I only tested it myself with Joy-Cons and a Pro Controller, but it should work with other listed controllers as well*

# macOS Installation and Usage 🍎

## Prerequisites

You need to have the following installed:

1. Xcode Command Line Tools
2. HIDAPI library

## Usage on macOS

1. Install prerequisites using Homebrew:
```bash
# Install the required packages
brew install hidapi

# If it says that 'brew' is unknown command, install Homebrew first:
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

2. Make sure your controller is connected via Bluetooth
   - To pair a new controller, press and hold the sync button (small round button) until the LEDs start flashing
   - Use macOS Bluetooth settings to complete the pairing

3. Run the downloaded program with:
```bash
./jcoff
```

4. Follow the on-screen instructions carefully

## Building on macOS

1. Clone this repository:
```bash
git clone https://github.com/manefunction/joycon-turnoff-ports.git
cd joycon-turnoff-ports
```

2. Run the build script:
```bash
chmod +x build-mac.sh
./build-mac.sh
```

The script will install required dependencies and build the project. After successful build, the binary will be available at `build/jcoff`.

## Troubleshooting on macOS

1. **Controller not detected**
   - Make sure the controller is connected via Bluetooth
   - Try disconnecting and reconnecting the controller
   - Ensure you have the necessary permissions to access HID devices

2. **Build fails**
   - Make sure all prerequisites are installed
   - Try running `brew update && brew upgrade`
   - Check if HIDAPI is properly installed with `brew info hidapi`

# Steam Deck Installation and Usage 🎮

## Prerequisites

You need to have the following packages installed:
1. HIDAPI library
2. libusb

These will be installed automatically by the installer or build script.

## Quick Install

1. Download and extract the latest release
2. Switch to Desktop Mode on your Steam Deck
3. Open Konsole (terminal)
4. Navigate to the extracted folder
5. Run the installer:
```bash
chmod +x deck-install.sh
./deck-install.sh
```

## Building from Source

1. Switch to Desktop Mode on your Steam Deck
2. Open Konsole (terminal)
3. Clone and build the project:
```bash
git clone https://github.com/manefunction/joycon-turnoff-ports.git
cd joycon-turnoff-ports
chmod +x build-deck.sh
./build-deck.sh
```

The script will install required dependencies and build the project. After successful build, the binary will be available at `build/jcoff`.

To install the built binary:
```bash
chmod +x deck-install.sh
./deck-install.sh
```

## Usage on Steam Deck

1. Make sure you're in Desktop Mode
2. Either:
   - Find "Joy-Con Shutdown Tool" in your applications menu, or
   - Open Konsole and type `jcoff`
3. Follow the on-screen instructions

The first time you run the tool, it will ask for your password to set up device permissions. This is normal and only needs to be done once.

## Troubleshooting on Steam Deck

If the controller is not detected:
1. Make sure you're in Desktop Mode
2. Make sure the controller is connected via Bluetooth
   - To pair: hold the sync button (small round button) until LEDs flash
   - Use Steam Deck's Bluetooth settings to complete pairing
3. Try disconnecting and reconnecting the controller

## Uninstalling from Steam Deck
```bash
rm -rf ~/.local/share/joycon-turnoff
rm ~/.local/bin/joycon-turnoff
rm ~/.local/share/applications/joycon-turnoff.desktop
```

# General Information

## After Shutdown

To wake up the controller after shutdown:
- Connect it to the Switch with a USB cable, or
- Perform Bluetooth pairing again by holding the sync button

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Credits

This is a macOS/Steam Deck port of the original Windows tool. The core functionality and protocol implementation remain the same.

For more info about the original tool, see: https://github.com/Sopsy/joycon-turnoff
