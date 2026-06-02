
#pragma once

#include "auth_repository.h"


class BootstrapService {
    public:
        static void try_bootstrap(IAuthRepository& auth_repo);
};

