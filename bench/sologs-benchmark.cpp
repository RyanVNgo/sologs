
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <algorithm>
#include <curl/curl.h>


struct Result {
    std::vector<long long> latencies_us;
    int success = 0;
    int failed = 0;
};

struct RequestContext {
    std::chrono::steady_clock::time_point start;
    std::string body;
};

static size_t discard_response(char* ptr, size_t size, size_t nmemb, void* userdata) {
    return size * nmemb;
}

long long now_us() {
    return std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now().time_since_epoch()
    ).count();
}

int main(int argc, char* argv[]) {
    std::string host = "127.0.0.1";
    int port = 8080;
    int total_requests = 10000;
    int concurrency = 100;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "--host" && i + 1 < argc) host = argv[++i];
        else if (arg == "--port" && i + 1 < argc) port = std::stoi(argv[++i]);
        else if (arg == "--requests" && i + 1 < argc) total_requests = std::stoi(argv[++i]);
        else if (arg == "--concurrency" && i + 1 < argc) concurrency = std::stoi(argv[++i]);
    }

    curl_global_init(CURL_GLOBAL_ALL);

    CURLM* multi = curl_multi_init();

    Result result;

    int in_flight = 0;
    int sent = 0;
    int completed = 0;

    auto start_all = std::chrono::steady_clock::now();

    auto make_request = [&](int id) -> CURL* {
        CURL* easy = curl_easy_init();

        std::string url = "http://" + host + ":" + std::to_string(port) + "/logs";

        auto* ctx = new RequestContext();
        ctx->start = std::chrono::steady_clock::now();
        ctx->body = R"({"message": "test log", "level": "INFO", "source": "sologs-benchmark"})";

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(easy, CURLOPT_URL, url.c_str());
        curl_easy_setopt(easy, CURLOPT_POSTFIELDS, ctx->body.c_str());
        curl_easy_setopt(easy, CURLOPT_POSTFIELDSIZE, ctx->body.size());
        curl_easy_setopt(easy, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, discard_response);

        curl_easy_setopt(easy, CURLOPT_PRIVATE, ctx);

        return easy;
    };

    for (int i = 0; i < concurrency && sent < total_requests; i++) {
        CURL* easy = make_request(sent++);
        curl_multi_add_handle(multi, easy);
        in_flight++;
    }

    int still_running = 0;
    curl_multi_perform(multi, &still_running);

    while (still_running || in_flight > 0) {
        int numfds;
        curl_multi_wait(multi, nullptr, 0, 1000, &numfds);

        curl_multi_perform(multi, &still_running);

        CURLMsg* msg;
        int msgs_left;

        while ((msg = curl_multi_info_read(multi, &msgs_left))) {
            if (msg->msg == CURLMSG_DONE) {
                CURL* easy = msg->easy_handle;

                RequestContext* ctx;
                curl_easy_getinfo(easy, CURLINFO_PRIVATE, &ctx);

                auto end = std::chrono::steady_clock::now();
                auto latency = std::chrono::duration_cast<std::chrono::microseconds>(
                    end - ctx->start
                ).count();

                result.latencies_us.push_back(latency);

                if (msg->data.result == CURLE_OK) {
                    result.success++;
                } else {
                    result.failed++;
                }

                delete ctx;
                curl_multi_remove_handle(multi, easy);
                curl_easy_cleanup(easy);

                in_flight--;
                completed++;

                if (sent < total_requests) {
                    CURL* next = make_request(sent++);
                    curl_multi_add_handle(multi, next);
                    in_flight++;
                }
            }
        }
    }

    auto end_all = std::chrono::steady_clock::now();

    double secs = std::chrono::duration<double>(end_all - start_all).count();
    double rps = result.success / secs;

    std::sort(result.latencies_us.begin(), result.latencies_us.end());

    auto p50 = result.latencies_us[result.latencies_us.size() * 0.50];
    auto p95 = result.latencies_us[result.latencies_us.size() * 0.95];
    auto p99 = result.latencies_us[result.latencies_us.size() * 0.99];

    std::cout << "Requests: " << total_requests << "\n";
    std::cout << "Success: " << result.success << "\n";
    std::cout << "Failed: " << result.failed << "\n";
    std::cout << "Concurrency: " << concurrency << "\n\n";

    std::cout << "Time: " << secs << "s\n";
    std::cout << "RPS: " << rps << "\n\n";

    std::cout << "Latency (us):\n";
    std::cout << "  p50: " << p50 << "\n";
    std::cout << "  p95: " << p95 << "\n";
    std::cout << "  p99: " << p99 << "\n";

    curl_multi_cleanup(multi);
    curl_global_cleanup();

    return 0;
}

