
#pragma once

#include "auth_repository.h"


class BootstrapService {
    public:
        static auto try_bootstrap(IAuthRepository& auth_repo) -> void;
};

