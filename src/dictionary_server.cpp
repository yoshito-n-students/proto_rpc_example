#include <map>
#include <string>

#include <boost/asio/io_service.hpp>
#include <boost/make_shared.hpp>

#include <proto_rpc/server.hpp>

#include <dictionary_service.pb.h>

namespace ba = boost::asio;
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

  ba::io_service queue;
  pr::Server server(queue, 12345, boost::make_shared< Service >());
  server.start();

  queue.run();

  return 0;
}