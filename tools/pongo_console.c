// pongo_console.c -- speaks the PongoOS console protocol (same one palera1n uses)
// usage: ./pongo_console [command] [seconds]   (no command: just read the console for N seconds)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <libusb-1.0/libusb.h>

int main(int argc, char** argv) {
    libusb_context* ctx = NULL;
    libusb_device_handle* h = NULL;
    if (libusb_init(&ctx) != 0) { fprintf(stderr, "libusb_init failed\n"); return 1; }
    h = libusb_open_device_with_vid_pid(ctx, 0x05ac, 0x4141);
    if (!h) { fprintf(stderr, "PongoOS device (05ac:4141) not found\n"); return 1; }
    libusb_detach_kernel_driver(h, 0);
    libusb_set_configuration(h, 1);
    libusb_claim_interface(h, 0);
    printf("[*] connected to PongoOS\n"); fflush(stdout);

    char buf[0x1000];

    // initial drain
    usleep(200000);
    for (int i = 0; i < 4; i++) {
        unsigned char inprog = 1;
        int r = libusb_control_transfer(h, 0xA1, 2, 0, 0, &inprog, 1, 1000);
        if (r < 0) break;
        r = libusb_control_transfer(h, 0xA1, 1, 0, 0, (unsigned char*)buf, 0x1000, 1000);
        if (r > 0) { fwrite(buf, 1, r, stdout); fflush(stdout); }
        if (!inprog) break;
    }

    if (argc > 1) {
        char cmd[512];
        snprintf(cmd, sizeof(cmd), "%s\n", argv[1]);
        int r = libusb_control_transfer(h, 0x21, 4, 1, 0, NULL, 0, 1000);
        if (r < 0) fprintf(stderr, "[!] ctrl(0x21/4) err %d\n", r);
        r = libusb_control_transfer(h, 0x21, 3, 0, 0, (unsigned char*)cmd, strlen(cmd), 3000);
        if (r < 0) fprintf(stderr, "[!] ctrl(0x21/3) err %d\n", r);
        printf("\n[[sent: %s]]\n", argv[1]); fflush(stdout);
    }

    int secs = argc > 2 ? atoi(argv[2]) : 15;
    time_t end = time(NULL) + secs;
    while (time(NULL) < end) {
        unsigned char inprog = 1;
        int r = libusb_control_transfer(h, 0xA1, 2, 0, 0, &inprog, 1, 1000);
        if (r >= 0) {
            r = libusb_control_transfer(h, 0xA1, 1, 0, 0, (unsigned char*)buf, 0x1000, 1000);
            if (r > 0) { fwrite(buf, 1, r, stdout); fflush(stdout); }
        }
        usleep(50000);
    }
    printf("\n[*] capture done\n");
    return 0;
}
