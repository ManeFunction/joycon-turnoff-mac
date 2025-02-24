#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <iomanip>
#include <thread>

#ifdef __APPLE__
#include <unistd.h>
#else
#include <cstdint>
#include <unistd.h>
#include <fcntl.h>
#endif

#include "hidapi.h"

#define VERSION "2.1.0"
#define HID_BUFFER_LENGTH 0x40
#define MAX_STR 10000

// VID should always be the same for first party controllers
#define NINTENDO_VID (0x57e)
#define JOY_CON_L_PID (0x2006)
#define JOY_CON_R_PID (0x2007)
#define PRO_CONTROLLER_PID (0x2009)
#define SNES_CONTROLLER_PID (0x2017)
#define N64_CONTROLLER_PID (0x2019)
#define CHARGING_GRIP_PID (0x200e)

#define CMD_SET_INPUT_REPORT_MODE (0x03)
#define CMD_SET_HCI_STATE (0x06)
#define CMD_RESET_PAIRING_INFO (0x07)
#define CMD_SET_SHIPMENT_LOW_POWER_STATE (0x08)
#define CMD_SET_LEDS (0x30)

bool enable_traffic_dump = false;
bool bluetooth = true;
uint8_t global_count = 0;
hid_device* handle;
unsigned char buf[HID_BUFFER_LENGTH];
int device_type = 0;

#ifndef __APPLE__
bool init_bluetooth_connection(hid_device* handle) {
    if (!handle) return false;
    
    // Set non-blocking mode for initial communication
    hid_set_nonblocking(handle, 1);
    
    // Quick drain of pending data (max 5 attempts)
    unsigned char temp_buf[64];
    int drain_attempts = 0;
    while (hid_read_timeout(handle, temp_buf, sizeof(temp_buf), 10) > 0 && drain_attempts++ < 5) {
        // Drain any pending data with shorter timeout
    }
    
    // Set blocking mode back
    hid_set_nonblocking(handle, 0);
    
    // Send initial handshake
    unsigned char handshake[8] = {0x80, 0x02};
    int res = hid_write(handle, handshake, sizeof(handshake));
    if (res < 0) {
        std::cerr << "Handshake failed: " << hid_error(handle) << std::endl;
        return false;
    }
    
    // Wait for response with shorter timeout
    res = hid_read_timeout(handle, temp_buf, sizeof(temp_buf), 500);
    if (res < 0) {
        std::cerr << "No response to handshake: " << hid_error(handle) << std::endl;
        return false;
    }
    
    return true;
}
#endif

void show_error(const char* title, unsigned char* buf, int length) {
    std::cerr << "Error: " << title << "\n\nBytes read/written: " << length << "\nData:\n";
    for (int i = 0; i < HID_BUFFER_LENGTH; i++) {
        fprintf(stderr, "%02X ", buf[i]);
    }
    std::cerr << std::endl;
}

void joycon_send_subcommand(hid_device* handle, int subcommand, uint8_t* data = nullptr, int data_length = 0) {
    memset(buf, 0, HID_BUFFER_LENGTH);

    buf[0] = 0x01; // Command
    buf[1] = (global_count++) & 0xF; // Packet number

    uint8_t rumble_base[8] = { 0x00, 0x01, 0x40, 0x40, 0x00, 0x01, 0x40, 0x40 };
    memcpy(buf + 2, rumble_base, 8);

    buf[10] = subcommand;
    if (data && data_length > 0) {
        memcpy(buf + 11, data, data_length);
    }

    int res = hid_write(handle, buf, HID_BUFFER_LENGTH);
    if (res == -1) {
        show_error("HID write error when sending a command!", buf, res);
        return;
    }

    usleep(100000); // 100ms sleep instead of Windows Sleep(100)

    if (subcommand == CMD_SET_HCI_STATE && (!data || data_length == 0)) {
        return;
    }

    memset(buf, 0, HID_BUFFER_LENGTH);
    res = hid_read_timeout(handle, buf, HID_BUFFER_LENGTH, 1000);
    if (res == -1) {
        show_error("HID read error when sending a command!", buf, res);
    }
}

void set_leds(hid_device* handle, uint8_t status) {
    uint8_t data[1] = { status };
    joycon_send_subcommand(handle, CMD_SET_LEDS, data, 1);
    usleep(50000); // Short delay to ensure LED state is set
}

#ifndef __APPLE__
void maintain_led_state(hid_device* handle, bool& should_continue) {
    while (should_continue) {
        set_leds(handle, 0b11110000);
        usleep(300000);  // 300ms on
        if (!should_continue) break;
        set_leds(handle, 0b00000000);
        usleep(300000);  // 300ms off
    }
}
#endif

void set_shipment_low_power(hid_device* handle) {
    uint8_t enable[1] = { 0x01 };
    joycon_send_subcommand(handle, CMD_SET_SHIPMENT_LOW_POWER_STATE, enable, 1);
}

void reset_pairing_information(hid_device* handle) {
    joycon_send_subcommand(handle, CMD_RESET_PAIRING_INFO);
}

void turn_off(hid_device* handle) {
    joycon_send_subcommand(handle, CMD_SET_HCI_STATE);
}

void set_input_report_mode(hid_device* handle) {
    uint8_t simple_hid_mode[1] = { 0x3f };
    joycon_send_subcommand(handle, CMD_SET_INPUT_REPORT_MODE, simple_hid_mode, 1);
}

int device_connection() {
    std::cout << "Looking for Nintendo Switch controllers...\n";
#ifndef __APPLE__ // SteamOS (Linux)
    struct hid_device_info *devs, *cur_dev;
    
    devs = hid_enumerate(NINTENDO_VID, 0x0);
    if (!devs) {
        std::cout << "No Nintendo devices found.\n";
        return 0;
    }
    
    // Store all potential Nintendo device paths
    std::vector<std::pair<std::string, uint16_t>> nintendo_paths;
    cur_dev = devs;
    while (cur_dev) {
        // Only store Nintendo devices with matching VID
        if (cur_dev->vendor_id == NINTENDO_VID) {
            nintendo_paths.push_back({cur_dev->path, cur_dev->product_id});
        }
        cur_dev = cur_dev->next;
    }
    hid_free_enumeration(devs);
    
    // Try opening each Nintendo device path
    for(const auto& path_info : nintendo_paths) {
        const auto& path = path_info.first;
        const auto& pid = path_info.second;
        
        handle = hid_open_path(path.c_str());
        if (!handle) {
            handle = hid_open(NINTENDO_VID, pid, nullptr);
        }
        
        if (handle) {
            // Initialize Bluetooth connection
            if (!init_bluetooth_connection(handle)) {
                hid_close(handle);
                continue;
            }
            
            // Successfully opened device
            switch(pid) {
                case JOY_CON_L_PID:
                    device_type = 1;
                    return 1;
                case JOY_CON_R_PID:
                    device_type = 2;
                    return 2;
                case PRO_CONTROLLER_PID:
                    device_type = 3;
                    return 3;
                case SNES_CONTROLLER_PID:
                    device_type = 4;
                    return 4;
                case N64_CONTROLLER_PID:
                    device_type = 5;
                    return 5;
                default:
                    hid_close(handle);
                    break;
            }
        }
    }
#else // macOS
    if ((handle = hid_open(NINTENDO_VID, JOY_CON_L_PID, nullptr))) {
        device_type = 1;
        return 1;
    }
    if ((handle = hid_open(NINTENDO_VID, JOY_CON_R_PID, nullptr))) {
        device_type = 2;
        return 2;
    }
    if ((handle = hid_open(NINTENDO_VID, PRO_CONTROLLER_PID, nullptr))) {
        device_type = 3;
        return 3;
    }
    if ((handle = hid_open(NINTENDO_VID, SNES_CONTROLLER_PID, nullptr))) {
        device_type = 4;
        return 4;
    }
    if ((handle = hid_open(NINTENDO_VID, N64_CONTROLLER_PID, nullptr))) {
        device_type = 5;
        return 5;
    }
#endif

    return 0;
}

const char* get_controller_name(int device_type) {
    switch (device_type) {
        case 1: return "Joy-Con (L)";
        case 2: return "Joy-Con (R)";
        case 3: return "Pro Controller";
        case 4: return "SNES Controller";
        case 5: return "N64 Controller";
        default: return "Unknown controller";
    }
}

std::string get_device_info() {
    wchar_t wmfg[MAX_STR];
    wchar_t wprod[MAX_STR];
    wchar_t wsn[MAX_STR];
    
    hid_get_manufacturer_string(handle, wmfg, MAX_STR);
    hid_get_product_string(handle, wprod, MAX_STR);
    hid_get_serial_number_string(handle, wsn, MAX_STR);
    
    char mfg[MAX_STR], prod[MAX_STR], sn[MAX_STR];
    wcstombs(mfg, wmfg, MAX_STR);
    wcstombs(prod, wprod, MAX_STR);
    wcstombs(sn, wsn, MAX_STR);
    
    return std::string(mfg) + " " + prod + "\nType: " + get_controller_name(device_type) + 
           "\nHID serial number: " + sn;
}

bool get_user_confirmation(const std::string& message) {
    std::cout << message << " (y/n): ";
    std::string response;
    std::getline(std::cin, response);
    return response == "y" || response == "Y";
}

int main(int argc, char* argv[]) {
    // Handle command line arguments
    if (argc > 1) {
        std::string arg(argv[1]);
        if (arg == "--help" || arg == "-h") {
            std::cout << "Joy-Con Shutdown Tool v" << VERSION << "\n"
                     << "A tool to turn off Nintendo Switch controllers and put them in factory state.\n\n"
                     << "Usage: " << argv[0] << " [OPTION]\n\n"
                     << "Options:\n"
                     << "  -h, --help     Display this help message\n"
                     << "  -v, --version  Display version information\n\n"
                     << "Supported controllers:\n"
                     << "  - Joy-Cons\n"
                     << "  - Pro Controller\n"
                     << "  - NES Joy-Cons\n"
                     << "  - SNES Controller\n"
                     << "  - N64 Controller\n\n"
                     << "For more information, visit: https://github.com/manefunction/joycon-turnoff-ports\n";
            return 0;
        }
        if (arg == "--version" || arg == "-v") {
            std::cout << "v" << VERSION << std::endl;
            return 0;
        }
    }

    if (!get_user_confirmation(
        "Please read all of this, this is important!\n\n"
        "WARNING: This program will write to the SPI flash of your connected Nintendo Switch controller.\n"
        "This can brick your controller.\n\n"
        "!!CONTINUE AT YOUR OWN RISK!!\n\n"
        "IMPORTANT: The controller needs to be connected via Bluetooth or this will not work!\n\n"
        "If all goes well, your controller will be left in a state like it was sent from the factory:\n"
        "- Low battery consumption (will not die in a week)\n"
        "- Pressing buttons will not turn it on\n"
        "- Pairing information is removed\n\n"
        "To reconnect the controller, do Bluetooth pairing again or connect it to your Switch with a USB cable.\n\n"
        "To start Bluetooth pairing, long-press the sync button (small round button near the LEDs)\n\n"
        "When controller will be connected to your device, proceed to continue. Continue?")) {
        return 1;
    }

    // Initialize HIDAPI
    if (hid_init() != 0) {
        std::cerr << "Failed to initialize HIDAPI" << std::endl;
        return 1;
    }

    while (!device_connection()) {
        if (!get_user_confirmation(
            "No Nintendo Switch controller was detected!\n\n"
            "Please connect the controller with Bluetooth and press 'y' to retry.\n"
            "Long press the sync button on the controller to turn on pairing mode.\n"
            "Press 'n' to cancel.")) {
            hid_exit();
            return 1;
        }
    }

    set_input_report_mode(handle);
    
#ifndef __APPLE__
    // For SteamOS, use a separate thread to maintain LED state
    // This is kinda clunky, but standard way is keep reseting the LEDs
    // after a few blinks because of the poor connection maintenance
    bool keep_blinking = true;
    std::thread blink_thread([&]() {
        maintain_led_state(handle, keep_blinking);
    });
#else
    // On macOS, single set is sufficient
    set_leds(handle, 0b11110000);
#endif

    std::string info_message = "Controller found!\n\n" + get_device_info() + 
        "\n\nPlease confirm that all the LEDs on the controller you want to turn off are flashing at the same time. "
        "If they are not, please cancel now and verify you are connecting the controller with Bluetooth!"
        "\n\nDo you want to turn the controller off?";

    bool confirmed = get_user_confirmation(info_message);

#ifndef __APPLE__
    keep_blinking = false;
    if (blink_thread.joinable()) {
        blink_thread.join();
    }
#endif

    if (!confirmed) {
        set_leds(handle, 0b00000001);
        hid_close(handle);
        hid_exit();
        return 1;
    }

    set_shipment_low_power(handle);
    reset_pairing_information(handle);
    turn_off(handle);

    std::cout << "Controller has been turned off." << std::endl;

    hid_close(handle);
    hid_exit();
    return 0;
} 