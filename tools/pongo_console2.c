// pongo_console2.c -- single connection: ';'-separated commands + continuous capture
// usage: ./pongo_console2 "cmd1;cmd2" capture_seconds
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <libusb-1.0/libusb.h>

static libusb_device_handle* h;

static void drain(int loops) {
    char buf[0x1000];
    for (int i = 0; i < loops; i++) {
        unsigned char inprog = 1;
        int r = libusb_control_transfer(h, 0xA1, 2, 0, 0, &inprog, 1, 800);
        if (r < 0) break;
        r = libusb_control_transfer(h, 0xA1, 1, 0, 0, (unsigned char*)buf, 0x1000, 800);
        if (r > 0) { fwrite(buf, 1, r, stdout); fflush(stdout); }
        if (!inprog) break;
    }
}

static int send_cmd(const char* c) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "%s\n", c);
    for (int t = 0; t < 15; t++) {
        int r = libusb_control_transfer(h, 0x21, 4, 1, 0, NULL, 0, 1000);
        if (r >= 0) r = libusb_control_transfer(h, 0x21, 3, 0, 0, (unsigned char*)cmd, strlen(cmd), 3000);
        if (r >= 0) { printf("\n[[sent: %s]]\n", c); fflush(stdout); return 0; }
        usleep(250000);
    }
    printf("\n[[FAILED to send: %s]]\n", c); fflush(stdout);
    return -1;
}

int main(int argc, char** argv) {
    libusb_context* ctx = NULL;
    if (libusb_init(&ctx) != 0) return 1;
    h = libusb_open_device_with_vid_pid(ctx, 0x05ac, 0x4141);
    if (!h) { fprintf(stderr, "PongoOS (4141) not found\n"); return 1; }
    libusb_detach_kernel_driver(h, 0);
    libusb_set_configuration(h, 1);
    libusb_claim_interface(h, 0);
    printf("[*] connected\n"); fflush(stdout);
    usleep(150000);
    drain(4);
    if (argc > 1) {
        char* cmds = strdup(argv[1]);
        char* save = NULL;
        for (char* c = strtok_r(cmds, ";", &save); c; c = strtok_r(NULL, ";", &save)) {
            send_cmd(c);
            usleep(100000);
            drain(1);
        }
        free(cmds);
    }
    int secs = argc > 2 ? atoi(argv[2]) : 30;
    time_t end = time(NULL) + secs;
    while (time(NULL) < end) {
        unsigned char inprog = 1;
        int r = libusb_control_transfer(h, 0xA1, 2, 0, 0, &inprog, 1, 800);
        if (r >= 0) {
            char buf[0x1000];
            r = libusb_control_transfer(h, 0xA1, 1, 0, 0, (unsigned char*)buf, 0x1000, 800);
            if (r > 0) { fwrite(buf, 1, r, stdout); fflush(stdout); }
        }
        usleep(50000);
    }
    printf("\n[*] capture done\n");
    return 0;
}
