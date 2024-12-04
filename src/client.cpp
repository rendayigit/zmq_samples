#include <iostream>
#include <string>
#include <thread>
#include <zmq.hpp>
#include <zmq_addon.hpp>

int main() {
  // Create a context
  zmq::context_t context(1);

  // Create a socket of type REQ
  zmq::socket_t subscriber(context, zmq::socket_type::sub);
  subscriber.connect("tcp://localhost:5555"); // Connect to the server

  // Subscribe to A envelope
  subscriber.set(zmq::sockopt::subscribe, "A");
  subscriber.set(zmq::sockopt::subscribe, "TERMINAL");

  while (true) {
    // Receive all parts of the message
    std::vector<zmq::message_t> recvMsgs;
    zmq::recv_result_t result = zmq::recv_multipart(subscriber, std::back_inserter(recvMsgs));
    assert(result && "recv failed");
    assert(*result == 2);

    std::cout << "Client received: [" << recvMsgs[0].to_string() << "] " << recvMsgs[1].to_string() << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // subscriber.send(zmq::str_buffer("OK!"), zmq::send_flags::dontwait);
  }

  return 0;
}
