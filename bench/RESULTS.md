
# Performance Results


## c0093f1 - 2026-04-17

### Summary
- Solid throughput on POST hot path
- Tail latency degrades as concurrency increases
- More timeouts as concurrency increases
- Long DB write times

### Environment
- Machine:
    - Intel i5-10400
    - 16 GB DDR4 RAM
    - 1 TB WD Black SN770 NVMe SSD
- OS: Linux Mint 22.3
- Build: Release
- Connection Type: Local Machine

### Benchmark Setup
- Tool: bombardier
- Duration: 10s
- Concurrency: 32, 64, 128, 256, 384, 512
- Flags: -m POST -f test_body.json -c ### -d 10s -l http://localhost:8080/logs
- Test JSON:
```json
{
    "message": "test log",
    "level": "INFO",
    "source": "sologs-benchmark"
}
```

### Results

#### Making POST Requests on /logs
##### RPS and Latency
| Conc. | RPS       | P50 (us)  | P95 (us)  | P99 (us)  | Timeouts |
| ----- | --------- | --------- | --------- | --------- | -------- |
| 32    | 165144    | 146       | 421       | 762       | 0        |
| 64    | 167606    | 183       | 633       | 2590      | 20       |
| 128   | 170814    | 187       | 625       | 2360      | 108      |
| 256   | 172816    | 209       | 655       | 3980      | 212      |
| 384   | 171397    | 204       | 676       | 70400     | 339      |
| 512   | 171636    | 207       | 665       | 118770    | 467      |

##### Database Writes
| Conc. | Writes    | Write Time (s) |
| ----- | --------- | -------------- |
| 32    | 1654656   | ~25            |
| 64    | 1679784   | ~33            |
| 128   | 1711642   | ~25            |
| 256   | 1731869   | ~28            |
| 384   | 1717896   | ~27            |
| 512   | 1719440   | ~33            |


## 2edd941 (enhancement/to-drogon) - 2026-06-04

### Changes
- switched from httplib to Drogon for API surface

### Summary
- consistent latency growth on increased concurrency across P values
- Zero timeouts across tested concurrency
- ~50% increase in RPS ingest throughput compared to previous impl

### Environment
- Machine:
    - Intel i5-10400
    - 16 GB DDR4 RAM
    - 1 TB WD Black SN770 NVMe SSD
- OS: Linux Mint 22.3
- Build: Release
- Connection Type: Local Machine

### Benchmark Setup
- Tool: bombardier
- Duration: 10s
- Concurrency: 32, 64, 128, 256, 384, 512
- Flags: -m POST -f test_body.json -c ### -d 10s -l http://localhost:8080/logs
- Test JSON:
```json
{
    "message": "test log",
    "level": "INFO",
    "source": "sologs-benchmark"
}
```

### Results

#### Making POST Requests on /logs
##### RPS and Latency
| Conc. | RPS       | P50 (us)  | P95 (us)  | P99 (us)  | Timeouts |
| ----- | --------- | --------- | --------- | --------- | -------- |
| 32    | 225343    | 106       | 335       | 609       | 0        |
| 64    | 258308    | 156       | 688       | 1400      | 0        |
| 128   | 270749    | 274       | 1430      | 2940      | 0        |
| 256   | 276461    | 523       | 3002      | 5007      | 0        |
| 384   | 271293    | 803       | 4490      | 7009      | 0        |
| 512   | 259480    | 1180      | 6000      | 9030      | 0        |


## c706cb0 - 2026-06-27

### Changes 
- no changes, re-evaluating performance with higher concurrency counts

### Summary
- slight regression in performance

### Benchmark Setup
- Tool: bombardier
- Duration: 10s
- Concurrency: 32, 64, 128, 256, 384, 512, 1024, 2048, 4096, 8192, 16384
- Flags: -m POST -f test_body.json -c ### -d 10s -l http://localhost:8080/logs
- Test JSON:
```json
{
    "message": "test log",
    "level": "INFO",
    "source": "sologs-benchmark"
}
```

### Results
#### Making POST Requests on /logs
##### RPS and Latency
| Conc. | RPS       | P50 (us)  | P95 (us)  | P99 (us)  | Timeouts |
| ----- | --------- | --------- | --------- | --------- | -------- |
| 32    | 205954    | 120       | 365       | 657       | 0        |
| 64    | 237393    | 162       | 781       | 1460      | 0        |
| 128   | 251197    | 297       | 1580      | 2940      | 0        |
| 256   | 232510    | 607       | 3560      | 5770      | 0        |
| 384   | 255504    | 870       | 4700      | 7230      | 0        |
| 512   | 255867    | 1240      | 5910      | 8720      | 0        |
| 1024  | 247196    | 3370      | 9980      | 13740     | 0        |
| 2048  | 240877    | 7610      | 17880     | 23840     | 0        |
| 4096  | 236890    | 15980     | 32350     | 42660     | 0        |
| 8192  | 225584    | 33850     | 59480     | 82970     | 391      |
| 16384 | 182787    | 74470     | 237780    | 584490    | 44       |

