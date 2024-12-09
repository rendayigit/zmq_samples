#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
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
      // Simple publish
      publisher.send(zmq::str_buffer("A"), zmq::send_flags::sndmore);
      publisher.send(zmq::str_buffer("Message in A envelope"));

      publisher.send(zmq::str_buffer("A"), zmq::send_flags::sndmore);
      publisher.send(zmq::str_buffer("More A messages"));

      publisher.send(zmq::str_buffer("B"), zmq::send_flags::sndmore);
      publisher.send(zmq::str_buffer("Message in B envelope"));

      publisher.send(zmq::str_buffer("C"), zmq::send_flags::sndmore);
      publisher.send(zmq::str_buffer("Message in C envelope"));

      // Publish a JSON message
      publisher.send(zmq::str_buffer("JSON"), zmq::send_flags::sndmore);
      nlohmann::json request;
      request["a"] = 123;
      std::string jsonStr = request.dump();
      zmq::message_t jsonMessage(jsonStr.data(), jsonStr.size());
      publisher.send(jsonMessage, zmq::send_flags::none);

      // Publish terminal standard outputs
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
    // Populate terminal standard outputs
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
