
#include <database/database.h>
#include <database/log_repository.h>
#include <database/auth_repository.h>
#include <service/log_service.h>
#include <service/user_service.h>
#include <service/bootstrap_service.h>
#include <server/server.h>


int main(void) {
    try {
        int port = 8080;

        SQLiteDatabase log_db("./sologs.sqlite");
        SQLiteDatabase auth_db("./sologs-auth.sqlite");

        SqlLogRepository log_repo(log_db);
        SqlAuthRepository auth_repo(auth_db);

        BootstrapService::try_bootstrap(auth_repo);
        UserService auth_service(auth_repo);
        LogService log_service(log_repo);

        SOLogSServer server(
            log_service,
            auth_service
        );

        std::cout << "Listening on port " << port << '\n';
        server.start(port);

        server.stop();
        std::cout << "Server shutdown successful" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Startup failed: " << e.what() << '\n';
        return 1;
    }
    return 0;
}

