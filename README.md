This repository contains working code for load-balancer in C++.
Steps to run the load-balancer.
1) Compile load_balancer.cpp and server.cpp files. {g++ load_balancer.cpp -o lb -lpthread, g++ server.cpp -o server -lpthread}
2) Start different server instances with different port {./server <portNumber>}
3) Start load_balancer {./lb <load_balancer_port> <server_port1> <server_port2> <server_port3> ...}
4) Hit a request on localhost:<load_balancer_port> using postman
