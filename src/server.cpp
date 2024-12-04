#include <fstream>
#include <iostream>
#include <streambuf>
#include <string>
#include <thread>
#include <zmq.hpp>

static zmq::context_t context(1);
static zmq::socket_t publisher(context, zmq::socket_type::pub);
static unsigned long long counter = 0;
static std::ostringstream local;

void publisherThread() {
  while (true) {
    try {
      publisher.send(zmq::str_buffer("A"), zmq::send_flags::sndmore);
      publisher.send(zmq::str_buffer("Message in A envelope"));

      publisher.send(zmq::str_buffer("A"), zmq::send_flags::sndmore);
      publisher.send(zmq::str_buffer("More A messages"));

      publisher.send(zmq::str_buffer("B"), zmq::send_flags::sndmore);
      publisher.send(zmq::str_buffer("Message in B envelope"));

      publisher.send(zmq::str_buffer("C"), zmq::send_flags::sndmore);
      publisher.send(zmq::str_buffer("Message in C envelope"));

      if (not local.str().empty()) {
        publisher.send(zmq::str_buffer("TERMINAL"), zmq::send_flags::sndmore);
        zmq::message_t message(local.str().data(), local.str().size());
        publisher.send(message, zmq::send_flags::none);
        local.clear();
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    } catch (std::exception const &e) {
      continue; // Waiting for a connection
    }
  }
}

void counterThread() {
  while (true) {
    std::cout << counter++ << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}

int main() {
  publisher.bind("tcp://*:5555"); // Bind to port 5555

  std::cout << "Server is running on port 5555..." << std::endl;

  std::cout.rdbuf(local.rdbuf()); // Forward cout to local string stream

  std::thread counterThreadObj([&] { counterThread(); });
  std::thread publisherThreadObj([&]() { publisherThread(); });

  counterThreadObj.join();
  publisherThreadObj.join();

  return 0;
}
