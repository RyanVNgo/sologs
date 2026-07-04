
#include <format>

#include <benchmark/benchmark.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <service/user_service.h>


class AuthRepositoryMock : public IAuthRepository {
    public:
        MOCK_METHOD(void, insert, (const AuthorizationEntry&), (override));
        MOCK_METHOD(void, insert_batch, (const std::vector<AuthorizationEntry>&), (override));
        MOCK_METHOD(std::vector<AuthorizationEntry>, get_auth_entries, (const UserFilterParams& params), (override));
        MOCK_METHOD(std::optional<AuthorizationEntry>, get_by_key_hash, (const std::string&), (const override));
        MOCK_METHOD(bool, has_any_admin, (), (const override));
};

static void BM_UserService_authenticate(benchmark::State& state) {
    AuthRepositoryMock mock_repo;
    UserService user_service(mock_repo);

    EXPECT_CALL(mock_repo, get_by_key_hash)
        .Times(testing::AnyNumber())
        .WillRepeatedly(testing::Return(AuthorizationEntry{
            .expires_at = "9999-01-01 00:00:00",
            .is_valid = true
        }));

    std::vector<std::string> keys(state.range(0));
    for (size_t i = 0; i < state.range(0); i++) {
        keys.push_back(std::format("key{}", i));
    }
    size_t idx = 0;

    for (auto _ : state) {
        state.PauseTiming();
        idx = (idx == (keys.size() - 1)) ? 0 : idx + 1;
        state.ResumeTiming();
        (void)user_service.authenticate(keys[idx]);
    }
}
BENCHMARK(BM_UserService_authenticate)
    ->RangeMultiplier(2)
    ->Range(8, 64)
    ->Name("UserService::authenticate: # of unique keys");


static void BM_UserService_subject_has_permissions_anyof_valid(benchmark::State& state) {
    AuthRepositoryMock mock_repo;
    UserService user_service(mock_repo);
    User subject{"uuid", "test", {Permissions::Admin}};
    const auto check_perms = {
        Permissions::LogRead,
        Permissions::LogWrite,
        Permissions::LogDelete,
        Permissions::AuthRead,
        Permissions::AuthWrite,
        Permissions::AuthDelete,
    };

    for (auto _ : state) {
        (void)user_service.subject_has_permissions(
            subject,
            check_perms,
            PermissionMode::AnyOf
        );
    }
}
BENCHMARK(BM_UserService_subject_has_permissions_anyof_valid)
    ->Name("UserService::subject_has_permissions: AnyOf Valid");

static void BM_UserService_subject_has_permissions_anyof_invalid(benchmark::State& state) {
    AuthRepositoryMock mock_repo;
    UserService user_service(mock_repo);
    User subject{"uuid", "test", {Permissions::LogRead}};
    const auto check_perms = {
        Permissions::LogRead,
        Permissions::LogWrite,
        Permissions::LogDelete,
        Permissions::AuthRead,
        Permissions::AuthWrite,
        Permissions::AuthDelete,
        Permissions::Admin
    };

    for (auto _ : state) {
        (void)user_service.subject_has_permissions(
            subject,
            check_perms,
            PermissionMode::AnyOf
        );
    }
}
BENCHMARK(BM_UserService_subject_has_permissions_anyof_invalid)
    ->Name("UserService::subject_has_permissions: AnyOf invalid");

static void BM_UserService_subject_has_permissions_allof_valid(benchmark::State& state) {
    AuthRepositoryMock mock_repo;
    UserService user_service(mock_repo);
    User subject{
            "uuid",
            "test",
            {
                Permissions::LogRead,
                Permissions::LogWrite,
                Permissions::LogDelete,
                Permissions::AuthRead,
                Permissions::AuthWrite,
                Permissions::AuthDelete,
                Permissions::Admin
            }
    };
    const auto check_perms = {
        Permissions::LogRead,
        Permissions::LogWrite,
        Permissions::LogDelete,
        Permissions::AuthRead,
        Permissions::AuthWrite,
        Permissions::AuthDelete,
        Permissions::Admin
    };

    for (auto _ : state) {
        (void)user_service.subject_has_permissions(
            subject,
            check_perms,
            PermissionMode::AllOf
        );
    }
}
BENCHMARK(BM_UserService_subject_has_permissions_allof_valid)
    ->Name("UserService::subject_has_permissions: AllOf Valid");

static void BM_UserService_subject_has_permissions_allof_invalid(benchmark::State& state) {
    AuthRepositoryMock mock_repo;
    UserService user_service(mock_repo);
    User subject{
            "uuid",
            "test",
            {
                Permissions::LogRead,
                Permissions::LogWrite,
                Permissions::LogDelete,
                Permissions::AuthRead,
                Permissions::AuthWrite,
                Permissions::AuthDelete,
            }
    };
    const auto check_perms = {
        Permissions::LogRead,
        Permissions::LogWrite,
        Permissions::LogDelete,
        Permissions::AuthRead,
        Permissions::AuthWrite,
        Permissions::AuthDelete,
        Permissions::Admin
    };

    for (auto _ : state) {
        (void)user_service.subject_has_permissions(
            subject,
            check_perms,
            PermissionMode::AllOf
        );
    }
}
BENCHMARK(BM_UserService_subject_has_permissions_allof_invalid)
    ->Name("UserService::subject_has_permissions: AllOf invalid");

