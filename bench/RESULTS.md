
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

