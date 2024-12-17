#include <iostream>
#include <string>
#include <thread>
#include <zmq.hpp>
#include <zmq_addon.hpp>

const std::string HOST = "localhost";
const std::string PORT = "5555";

constexpr int THREAD_SLEEP_DURATION = 100;

int main() {
  // Create a context
  zmq::context_t context(1);

  // Create a socket of type REQ
  zmq::socket_t subscriber(context, zmq::socket_type::sub);

  // Connect to the server
  subscriber.connect("tcp://" + HOST + ":" + PORT);

  // Subscribe to A topic
  subscriber.set(zmq::sockopt::subscribe, "A");

  // Subscribe to TERMINAL topic
  subscriber.set(zmq::sockopt::subscribe, "TERMINAL");

  // Subscribe to JSON topic
  subscriber.set(zmq::sockopt::subscribe, "JSON");

  // Subscribe to JSON topic
  subscriber.set(zmq::sockopt::subscribe, "MANUAL");

  while (true) {
    // Receive all parts of the message
    std::vector<zmq::message_t> recvMsgs;
    zmq::recv_result_t result = zmq::recv_multipart(subscriber, std::back_inserter(recvMsgs));
    assert(result && "\n>>> recv failed");
    assert(*result == 2);

    std::cout << "\n>>> client received: " << std::endl;
    std::cout << ">>> TOPIC: " << recvMsgs.at(0).to_string() << ", MESSAGE: " << recvMsgs.at(1).to_string()
              << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(THREAD_SLEEP_DURATION));
  }

  return 0;
}
