#define _GNU_SOURCE  // Required for asprintf.
#include <assert.h>
#include <malloc.h>
#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/signal.h>
#include <time.h>
#include <unistd.h>

#include <ixp.h>

static bool running = true;
static struct sigaction sa;

static void signal_handler(int signal) {
    sa.sa_handler = SIG_DFL;
    sigaction(signal, &sa, NULL);

    running = false;
}

int main(int argc, char **argv) {
    char *client_address;
    IxpClient *client;
    IxpCFid *fid;
    time_t now;
    char *now_str;
    char *colors_str;
    char *buffer;
    int buffer_size;
    int uid;
    char *path;

    client_address = getenv("WMII_ADDRESS");
    if (client_address && client_address[0] != '\0') {
        client = ixp_mount(client_address);
    } else {
        client = ixp_nsmount("wmii");
    }
    if (client == NULL) {
        fprintf(stderr, "could not connect to wmii: %s\n", ixp_errbuf());
        return 1;
    }

    colors_str = getenv("WMII_NORMCOLORS");
    if (colors_str == NULL) {
        fprintf(stderr, "WMII_NORMCOLORS is not set");
        return 1;
    }

    // Generate a random identifier to distinguish this clock from all of the
    // other clocks.
    srand(time(NULL));
    uid = rand() % 0xffff;

    asprintf(&path, "/rbar/900-%04x-clock", uid);
    assert(path != NULL);

    fid = ixp_create(client, path, 0777, P9_OWRITE);
    if (fid == NULL) {
        fprintf(stderr, "could not create widget %s\n", ixp_errbuf());
        return 1;
    }

    buffer_size = asprintf(&buffer, "colors %s\n", colors_str);
    assert(buffer != NULL);
    ixp_write(fid, buffer, buffer_size - 1);

    sa.sa_flags = 0;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        return 1;
    }

    while (running) {
        now = time(NULL);
        now_str = ctime(&now);
        assert(now_str != NULL);

        buffer_size = asprintf(&buffer, "label %s\n", now_str);
        assert(buffer != NULL);

        ixp_write(fid, buffer, buffer_size - 1);

        free(buffer);
        sleep(1);
    }

    ixp_close(fid);
    ixp_remove(client, path);
    ixp_unmount(client);

    free(path);

    return 0;
}
