#include <map>
#include <string>

#include <boost/asio/io_service.hpp>
#include <boost/make_shared.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <boost/program_options/variables_map.hpp>

#include <proto_rpc/server.hpp>

#include <dictionary_service.pb.h>

namespace ba = boost::asio;
namespace bp = boost::program_options;
namespace gp = google::protobuf;
namespace pr = proto_rpc;

class Service : public dictionary::Service {
public:
  Service() {}

  void Set(gp::RpcController *controller, const dictionary::KeyValue *request,
           dictionary::Empty *response, gp::Closure *done) {
    std::cout << "Setting " << request->value() << " for " << request->key() << " ..." << std::endl;
    data_.insert(std::make_pair(request->key(), request->value()));
    done->Run();
  }

  void Get(gp::RpcController *controller, const dictionary::Key *request,
           dictionary::Value *response, gp::Closure *done) {
    std::cout << "Getting a value for " << request->key() << " ..." << std::endl;
    std::map< std::string, double >::const_iterator entry(data_.find(request->key()));
    if (entry == data_.end()) {
      response->clear_value();
    } else {
      response->set_value(entry->second);
    }
    done->Run();
  }

private:
  std::map< std::string, double > data_;
};

int main(int argc, char *argv[]) {

  // read parameters from the command line
  unsigned short port;
  try {
    // define available options
    bp::options_description options;
    options.add(boost::make_shared< bp::option_description >("help", bp::bool_switch()));
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
    port = args["port"].as< unsigned short >();
  } catch (const std::exception &error) {
    std::cerr << "Error: " << error.what() << std::endl;
    return 1;
  }

  ba::io_service queue;
  pr::Server server(queue, port, boost::make_shared< Service >());
  server.start();

  queue.run();

  return 0;
}