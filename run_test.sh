cd build
cmake ..
make
#./cppUtest_serial_start -g cppUtest_serial_local_rb -v -c
#./cppUtest_serial_start -g cppUtest_serial_local_hard_test -v -c
#./cppUtest_serial_start -g cppUtest_serial_A2B_test -v -c
./cppUtest_serial_start -g cppUtest_socket_easy_test -v -c
