
#include "db/database.h"
#include "db/log_repository.h"
#include "service/log_service.h"
#include "api/server.h"


std::atomic<bool> shutdown_requested = false;

void sig_handler(int sig) {
    shutdown_requested = true;
}

int main(void) {
    try {
        int port = 8080;

        SQLiteDatabase db("./sologs.sqlite");
        LogRepository log_repo(db);
        LogService log_service(log_repo);
        SOLogSServer server(log_service);

        std::signal(SIGINT, sig_handler);
        std::signal(SIGTERM, sig_handler);

        std::thread shutdown_watcher(
            [&]() {
                while (!shutdown_requested) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
                std::cout << "\nServer shutting down..." << std::endl;
                server.stop();
            }
        );

        std::cout << "Listening on port " << port << '\n';
        server.start(port);

        shutdown_watcher.join();

        std::cout << "Server shutdown successful" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Startup failed: " << e.what() << '\n';
        return 1;
    }
    return 0;
}

