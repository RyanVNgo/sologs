
# Performance Results


## c271cf6 - 2026-04-16

### Summary
- Performance is solid for an initial implementation
- Hits initial target of 30K RPS (writes), hitting ~47K RPS
- Steady latency degredation with increased connection concurrency
- Breaks down aggresively with too many concurrent connections

### Changes Since Last / Implementation Details
- first functional implementation that can take moderate load
- uses cpp-httplib for handling I/O
- log-writes are queued in service layer
- server layer worker thread does batch writes from queue
- queue safety is managed by single mutex locking

### Environment
- Machine:
    - Intel i5-10400
    - 16 GB DDR4 RAM
    - 1 TB WD Black SN770 NVMe SSD
- OS: Linux Mint 22.3
- Build: Release

### Benchmark Setup
- Requests: 128000
- Concurrency: 64 ~ 96
- Iterations: 5 Runs Averaged

### Results
| Concurrency   | RPS       | P50 (us)  | P95 (us)  | P99 (us)  |
| ------------- | --------- | --------- | --------- | --------- |
| 64            | 46846     | 1320      | 1626      | 2056      |
| 80            | 47483     | 1324      | 2004      | 2502      |
| 96            | 16405     | 1831      | 2343      | 3874      |

### Observations
- Latency tends to steadily worsen under greater concurrency
- Large drop / unstable performance beyond concurreny 80

