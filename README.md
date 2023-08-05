# stackless_concurrency

This is an implementation for serial lock-free mutex for stackless coroutines C++20. <br>
Lewis Baker, one of the creators of cpp coroutines, has [one](https://github.com/lewissbaker/cppcoro/blob/master/include/cppcoro/async_mutex.hpp) in his amazing library [cppcoro](https://github.com/lewissbaker/cppcoro). Unfortunately I didn't quite understand his thought process and the necessity of techniques that he used in his implementation, so I decided to find my own way to solve this task. My mutex uses one intrusive lock-free stack without the need of a second queue.<br>

Tests were conducted via wonderful testing framework for concurrency provided [here](https://gitlab.com/Lipovsky/concurrency-course).
