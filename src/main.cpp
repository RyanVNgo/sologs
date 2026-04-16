
#include "db/database.h"
#include "db/log_repository.h"
#include "service/log_service.h"
#include "api/server.h"


int main(void) {
    SQLiteDatabase db("./sologs.sqlite");
    LogRepository log_repo(db);
    LogService log_service(log_repo);
    SOLogSServer server(log_service);
    server.start(8080);
    return 0;
}

