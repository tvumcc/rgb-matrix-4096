#include "led-matrix.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <csignal>
#include <cstdint>
#include <cstring>
#include <iostream>

using rgb_matrix::RGBMatrix;
using rgb_matrix::FrameCanvas;

static const int WIDTH = 64;
static const int HEIGHT = 64;
static const size_t FRAME_SIZE = WIDTH * HEIGHT * 3;
static const int PORT = 5006;

volatile bool running = true;
static void InterruptHandler(int) { running = false; }

// Read exactly `len` bytes from a socket, or return false on disconnect/error.
static bool ReadFull(int fd, uint8_t *buf, size_t len) {
    size_t total = 0;
    while (total < len) {
        ssize_t n = read(fd, buf + total, len - total);
        if (n <= 0) return false;
        total += static_cast<size_t>(n);
    }
    return true;
}

int main(int argc, char *argv[]) {
    rgb_matrix::RGBMatrix::Options matrix_options;
    rgb_matrix::RuntimeOptions runtime_opt;

    if (!rgb_matrix::ParseOptionsFromFlags(&argc, &argv, &matrix_options,
                                            &runtime_opt)) {
        return 1;
    }

    RGBMatrix *matrix = RGBMatrix::CreateFromOptions(matrix_options, runtime_opt);
    if (matrix == nullptr) {
        std::cerr << "Failed to create matrix. Check wiring/flags.\n";
        return 1;
    }
    FrameCanvas *canvas = matrix->CreateFrameCanvas();

    signal(SIGTERM, InterruptHandler);
    signal(SIGINT, InterruptHandler);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, reinterpret_cast<sockaddr *>(&address), sizeof(address)) < 0) {
        std::cerr << "bind() failed on port " << PORT << "\n";
        return 1;
    }
    listen(server_fd, 5);
    std::cout << "Listening on port " << PORT << " for " << WIDTH << "x"
                << HEIGHT << " RGB frames...\n";

    uint8_t buffer[FRAME_SIZE];

    while (running) {
        pollfd pfd{server_fd, POLLIN, 0};
        int poll_result = poll(&pfd, 1, 500 /* ms */);
        if (poll_result <= 0) continue;
 

        int client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd < 0) continue;

        if (ReadFull(client_fd, buffer, FRAME_SIZE)) {
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
            size_t idx = (y * WIDTH + x) * 3;
            canvas->SetPixel(x, y, buffer[idx], buffer[idx + 1], buffer[idx + 2]);
            }
        }
        canvas = matrix->SwapOnVSync(canvas);
        } else {
        std::cerr << "Incomplete frame received, discarding.\n";
        }

        close(client_fd);
    }

    close(server_fd);
    matrix->Clear();
    delete matrix;
    return 0;
}