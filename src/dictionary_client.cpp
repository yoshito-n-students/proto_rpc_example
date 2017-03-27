#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/make_shared.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <boost/program_options/variables_map.hpp>

#include <proto_rpc/channel.hpp>
#include <proto_rpc/controller.hpp>

#include <dictionary_service.pb.h>

namespace ba = boost::asio;
namespace bp = boost::program_options;
namespace gp = google::protobuf;
namespace pr = proto_rpc;

int main(int argc, char *argv[]) {

  // read parameters from the command line
  ba::ip::address_v4 address;
  unsigned short port;
  try {
    // define available options
    bp::options_description options;
    options.add(boost::make_shared< bp::option_description >("help", bp::bool_switch()));
    options.add(boost::make_shared< bp::option_description >(
        "address", bp::value< std::string >()->default_value("127.0.0.1")));
    options.add(boost::make_shared< bp::option_description >(
        "port", bp::value< unsigned short >()->default_value(12345)));
    // parse the command line
    bp::variables_map args;
    bp::store(bp::parse_command_line(argc, argv, options), args);
    if (args["help"].as< bool >()) {
      std::cout << "Available options:\n" << options << std::endl;
      return 0;
    }
    // convert the parsing result
    address = ba::ip::address_v4::from_string(args["address"].as< std::string >());
    port = args["port"].as< unsigned short >();
  } catch (const std::exception &error) {
    std::cerr << "Error: " << error.what() << std::endl;
    return 1;
  }

  // construct a RPC client
  pr::Channel channel(address, port);
  dictionary::Service::Stub client(&channel);

  while (true) {
    // get a command
    std::cout << "Enter \"key value\" to set, or \"key\" to get: " << std::flush;
    std::string line;
    std::getline(std::cin, line);

    // parse the command to extract the key and value
    std::istringstream iss(line);
    std::string key;
    double value;
    iss >> key >> value;

    // run a RPC
    if (key.empty()) {
      // the key is invalid
      std::cerr << "Invalid key. Try again." << std::endl;
    } else if (!iss.fail()) {
      // both the key and value are valid. run Set().
      pr::Controller controller;
      dictionary::KeyValue request;
      dictionary::Empty response;
      request.set_key(key);
      request.set_value(value);
      client.Set(&controller, &request, &response, NULL);
      if (controller.Failed()) {
        std::cout << "Set: NG (" << controller.ErrorText() << ")" << std::endl;
      } else {
        std::cout << "Set: OK" << std::endl;
      }
    } else {
      // only the key is valid. run Get().
      pr::Controller controller;
      dictionary::Key request;
      dictionary::Value response;
      request.set_key(key);
      client.Get(&controller, &request, &response, NULL);
      if (controller.Failed()) {
        std::cout << "Get: NG (" << controller.ErrorText() << ")" << std::endl;
      } else {
        if (response.has_value()) {
          std::cout << "Get: OK (" << response.value() << ")" << std::endl;
        } else {
          std::cout << "Get: OK (<none>)" << std::endl;
        }
      }
    }
  }

  return 0;
}