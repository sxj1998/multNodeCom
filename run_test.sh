cd build
cmake ..
make
./cppUtest_serial_start -g cppUtest_serial_local_rb -v
./cppUtest_serial_start -g cppUtest_serial_local_hard_test -v
