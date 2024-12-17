#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <streambuf>
#include <string>
#include <thread>
#include <zmq.hpp>

constexpr int MESSAGING_MAX_COMMAND_SIZE = 1024;
constexpr int THREAD_SLEEP_DURATION = 1000;

const std::string PUBLISHER_PORT = "5555";
const std::string LOOPBACK_PORT = "5556";

static zmq::context_t context(1);
static zmq::socket_t publisher(context, zmq::socket_type::pub);

static zmq::context_t loopbackContext(1);
static zmq::socket_t loopback(loopbackContext, zmq::socket_type::pair);

static unsigned long long counter = 0;
static std::ostringstream local;

void loopbackThread() {
  while (true) {
    std::array<char, MESSAGING_MAX_COMMAND_SIZE> buf{'\0'};
    zmq::mutable_buffer request(buf.data(), buf.size());
    zmq::recv_buffer_result_t result = loopback.recv(request, zmq::recv_flags::none);

    std::string loopbackMessage = static_cast<char *>(request.data());
    publisher.send(zmq::str_buffer("LOOPBACK"), zmq::send_flags::sndmore);
    zmq::message_t zloopbackMessage(loopbackMessage.data(), loopbackMessage.size());
    publisher.send(zloopbackMessage, zmq::send_flags::none);
  }
}

void publisherThread() {
  while (true) {
    try {
      // Simple publish
      publisher.send(zmq::str_buffer("A"), zmq::send_flags::sndmore);
      publisher.send(zmq::str_buffer("message in A topic"));

      publisher.send(zmq::str_buffer("A"), zmq::send_flags::sndmore);
      publisher.send(zmq::str_buffer("more A messages"));

      publisher.send(zmq::str_buffer("B"), zmq::send_flags::sndmore);
      publisher.send(zmq::str_buffer("message in B topic"));

      publisher.send(zmq::str_buffer("C"), zmq::send_flags::sndmore);
      publisher.send(zmq::str_buffer("message in C topic"));

      // Publish a JSON message
      publisher.send(zmq::str_buffer("JSON"), zmq::send_flags::sndmore);
      nlohmann::json request;
      request["a"] = 1;
      std::string jsonStr = request.dump();
      zmq::message_t jsonMessage(jsonStr.data(), jsonStr.size());
      publisher.send(jsonMessage, zmq::send_flags::none);

      // Publish terminal standard outputs
      if (not local.str().empty()) {
        publisher.send(zmq::str_buffer("TERMINAL"), zmq::send_flags::sndmore);

        zmq::message_t message(local.str().data(), local.str().size());
        publisher.send(message, zmq::send_flags::none);

        // Clear terminal stream
        local.str("");
        local.clear();
      }
    } catch (std::exception const &e) {
      continue; // Waiting for a connection
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(THREAD_SLEEP_DURATION));
  }
}

void counterThread() {
  while (true) {
    // Populate terminal standard outputs
    std::cout << counter++ << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(THREAD_SLEEP_DURATION));
  }
}

void manualThread() {
  std::string input;

  while (true) {
    std::getline(std::cin, input);

    publisher.send(zmq::str_buffer("MANUAL"), zmq::send_flags::sndmore);
    zmq::message_t message(input.data(), input.size());
    publisher.send(message, zmq::send_flags::none);
  }
}

int main() {
  // Bind to port 5555
  publisher.bind("tcp://*:" + PUBLISHER_PORT);
  loopback.bind("tcp://*:" + LOOPBACK_PORT);

  std::cout << ">>> server is running on port " << PUBLISHER_PORT << std::endl;

  // Forward cout to local string stream
  std::cout.rdbuf(local.rdbuf());

  std::thread counterThreadObj([&] { counterThread(); });
  std::thread publisherThreadObj([&]() { publisherThread(); });
  std::thread pairThreadObj([&] { loopbackThread(); });
  std::thread manualThreadObj([&] { manualThread(); });

  counterThreadObj.join();
  publisherThreadObj.join();

  return 0;
}
