
#include "db/database.h"
#include "db/log_repository.h"
#include "service/log_service.h"
#include "api/server.h"


int main(void) {
    try {
        SQLiteDatabase db("./sologs.sqlite");
        LogRepository log_repo(db);
        LogService log_service(log_repo);
        SOLogSServer server(log_service);
        server.start(8080);
    } catch (const std::exception& e) {
        std::cerr << "Startup failed: " << e.what() << '\n';
        return 1;
    }
    return 0;
}

