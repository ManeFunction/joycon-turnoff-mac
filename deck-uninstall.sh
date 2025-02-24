#!/bin/bash

echo "Joy-Con Shutdown Tool uninstaller for Steam Deck"

# Remove binary and its directory
echo "Removing program files..."
rm -rf ~/.local/share/jcoff

# Remove launcher scripts
echo "Removing launcher scripts..."
rm -f ~/.local/bin/jcoff
rm -f ~/.local/bin/joycon-turnoff

# Remove desktop entry
echo "Removing desktop shortcut..."
rm -f ~/.local/share/applications/jcoff.desktop

echo "Uninstallation complete!"
echo "The Joy-Con Shutdown Tool has been removed from your system." 