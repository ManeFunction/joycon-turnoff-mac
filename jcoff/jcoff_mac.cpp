#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include "hidapi.h"

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
}

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
    // Joy-Con (L)
    if ((handle = hid_open(NINTENDO_VID, JOY_CON_L_PID, nullptr))) {
        device_type = 1;
        return 1;
    }

    // Joy-Con (R)
    if ((handle = hid_open(NINTENDO_VID, JOY_CON_R_PID, nullptr))) {
        device_type = 2;
        return 2;
    }

    // Pro Controller
    if ((handle = hid_open(NINTENDO_VID, PRO_CONTROLLER_PID, nullptr))) {
        device_type = 3;
        return 3;
    }

    // SNES Controller
    if ((handle = hid_open(NINTENDO_VID, SNES_CONTROLLER_PID, nullptr))) {
        device_type = 4;
        return 4;
    }

    // N64 Controller
    if ((handle = hid_open(NINTENDO_VID, N64_CONTROLLER_PID, nullptr))) {
        device_type = 5;
        return 5;
    }

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

int main() {
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
        "When controller will be connected to your Mac, proceed to continue. Continue?")) {
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
    set_leds(handle, 0b11110000);

    std::string info_message = "Controller found!\n\n" + get_device_info() + 
        "\n\nPlease confirm that all the LEDs on the controller you want to turn off are flashing at the same time. "
        "If they are not, please cancel now and verify you are connecting the controller with Bluetooth!"
        "\n\nDo you want to turn the controller off?";

    if (!get_user_confirmation(info_message)) {
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