#!/bin/bash

echo "Joy-Con Shutdown Tool installer for Steam Deck"
echo "This will install the tool for use in Desktop Mode"

# Check if running on SteamOS
if ! grep -q "ID=steamos" /etc/os-release 2>/dev/null; then
    echo "Warning: This doesn't seem to be SteamOS."
    echo "The installer is optimized for Steam Deck, but you can proceed at your own risk."
    read -p "Do you want to continue anyway? (y/N): " response
    if [[ ! "$response" =~ ^[Yy]$ ]]; then
        echo "Installation cancelled."
        exit 1
    fi
fi

# Install required dependencies
echo "Installing required dependencies..."
sudo pacman -S --needed --noconfirm hidapi libusb

# Create directories
echo "Creating directories..."
mkdir -p ~/.local/bin
mkdir -p ~/.local/share/jcoff

# Copy binary
echo "Installing binary..."
cp ./jcoff ~/.local/share/jcoff/
chmod +x ~/.local/share/jcoff/jcoff

# Create launcher script
echo "Creating launcher script..."
cat > ~/.local/bin/jcoff << 'EOF'
#!/bin/bash
exec "$HOME/.local/share/jcoff/jcoff" "$@"
EOF

chmod +x ~/.local/bin/jcoff

# Create symbolic link for compatibility
ln -sf ~/.local/bin/jcoff ~/.local/bin/joycon-turnoff

# Create desktop entry
echo "Creating desktop shortcut..."
cat > ~/.local/share/applications/jcoff.desktop << EOF
[Desktop Entry]
Name=Joy-Con Shutdown Tool
Comment=Turn off Nintendo Switch controllers
Exec=$HOME/.local/share/jcoff/jcoff
Terminal=true
Type=Application
Categories=Utility;Game;
Icon=input-gaming
EOF

# Add ~/.local/bin to PATH if not already there
if ! grep -q "export PATH=\"\$HOME/.local/bin:\$PATH\"" ~/.bashrc; then
    echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.bashrc
fi
if ! grep -q "export PATH=\"\$HOME/.local/bin:\$PATH\"" ~/.zshrc 2>/dev/null; then
    echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.zshrc 2>/dev/null
fi

# Reload desktop database
update-desktop-database ~/.local/share/applications &>/dev/null || true

echo "Installation complete!"
echo "You can now run the tool by typing 'jcoff' in the terminal"
echo "or find 'Joy-Con Shutdown Tool' in your applications menu"
echo -e "\nNOTE: You might need to log out and log back in for the command to work in terminal,"
echo "but you can use the full path right now: ~/.local/share/jcoff/jcoff" 