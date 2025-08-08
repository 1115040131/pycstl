```bash
$ ./make monkey_bench
usage: fibonacci -engine vm|eval [-builtin]

$ ./make monkey_bench -- -engine eval
engine=eval, fibonacci(35)=9227465, duration=12.092080939s

$ ./make monkey_bench -- -engine vm
engine=vm, fibonacci(35)=9227465, duration=11.851909546s

$ ./make monkey_bench -- -eval vm -builtin
engine=eval, fibonacci(35)=9227465, duration=0.007922004s

$ ./make monkey_bench -- -engine vm -builtin
engine=vm, fibonacci(35)=9227465, duration=0.007922004s

$ time python3 monkey/bench/fibonacci.py
python3 monkey/bench/fibonacci.py  0.60s user 0.00s system 99% cpu 0.599 total
```