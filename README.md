# cpp-toolbox

This is the cpp-toolbox project.

# Building and installing

See the [BUILDING](BUILDING.md) document.

# List of tools

- [base](https://github.com/nerdneilsfield/cpp-toolbox/tree/master/src/include/cpp-toolbox/base) some base classes
  - [env](https://github.com/nerdneilsfield/cpp-toolbox/tree/master/src/include/cpp-toolbox/base/env.hpp) environment variables related functions
  - [thread_pool] a thread pool class based on [lock_free_queue](https://github.com/nerdneilsfield/cpp-toolbox/tree/master/src/include/cpp-toolbox/container/lock_free_queue.hpp)
- [container](https://github.com/nerdneilsfield/cpp-toolbox/tree/master/src/include/cpp-toolbox/container) some container classes
  - [lock_free_queue](https://github.com/nerdneilsfield/cpp-toolbox/tree/master/src/include/cpp-toolbox/container/lock_free_queue.hpp) a lock free queue class
  - [string](https://github.com/nerdneilsfield/cpp-toolbox/tree/master/src/include/cpp-toolbox/container/string.hpp) lots of helper functions for `std::string` and `std::string_view`
- [file](https://github.com/nerdneilsfield/cpp-toolbox/tree/master/src/include/cpp-toolbox/file) some file related functions
- [logger](https://github.com/nerdneilsfield/cpp-toolbox/tree/master/src/include/cpp-toolbox/logger) some logger classes
- [utils](https://github.com/nerdneilsfield/cpp-toolbox/tree/master/src/include/cpp-toolbox/utils) some utility functions and classes
  - [timer](https://github.com/nerdneilsfield/cpp-toolbox/tree/master/src/include/cpp-toolbox/utils/timer.hpp) a stop watch class and some timer related functions