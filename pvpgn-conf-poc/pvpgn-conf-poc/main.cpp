#include <iostream>
#include <fstream>
#include <string>

//POCO
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/URI.h>
#include <cereal/archives/json.hpp>

using namespace Poco;
using namespace Poco::Net;
using namespace std;

class Config {
	
public:
	Config() {
		/*serveraddrs_doc = "This is a comma delimited list of hostnames that the server should listen on. It might be useful to make an internal-only server on a gateway machine for example...";
		serveraddrs = "eurobattle.net";
		w3routeaddr_doc = "W3 Play Game router address. Just put your server address in here or use 0.0.0.0:6200 for server to bind to all interfaces, but make sure you set up w3trans if you do.";
		w3routeaddr = "0.0.0.0:6200";
		test = 11111;*/
	}

	template<class Archive>
	void serialize(Archive & archive) {
		archive(CEREAL_NVP(serveraddrs_doc), 
				CEREAL_NVP(serveraddrs),
				CEREAL_NVP(w3routeaddr_doc),
				CEREAL_NVP(w3routeaddr),
				CEREAL_NVP(test)); // serialize things by passing them to the archive
	}

	string getServeraddrs() {
		return serveraddrs;
	}

	string serveraddrs_doc;
	string serveraddrs;
	string w3routeaddr_doc;
	string w3routeaddr;
	int test;

};

//request handler
class MyRequestHandler : public HTTPRequestHandler
{
public:
	virtual void handleRequest(HTTPServerRequest &req, HTTPServerResponse &resp) {
		
		cout << "Handling request";

		resp.setStatus(HTTPResponse::HTTP_OK);
		resp.setContentType("text/html");

		ostream& out = resp.send();

		URI uri(req.getURI());
		
		string path = uri.getPath();

		if (path == "/hello") {
			out << "hello world";
		}
		else if (path == "/config") {
			//deserialize from json file
			Config configInstance;
			{
				ifstream filein("C:/Users/klemen/git/pvpgn-conf-poc/pvpgn-conf-poc/config.json");
				if (filein.is_open()) {
					cout << "File opened, deserializing";
					cereal::JSONInputArchive iarchive(filein); // Create an input archive
					iarchive(configInstance); // Read the data from the archive
					//debug, check if config data was filled by deserializer
					cout << configInstance.getServeraddrs();
				}
				else {
					cout << "Cant read config file";
					return;
				}
			}

			//serialize config object to json
			//streams must be scoped {n so they autoflush on close
			stringstream ssout; // any stream can be used
			{
				cout << "Serializing";
				cereal::JSONOutputArchive oarchive(ssout); // Create an output archive
				oarchive(cereal::make_nvp("config", configInstance)); // Write the data to the archive
				ssout.flush();
			}

			//send response
			out << "<h1>Hello world!</h1>"
				<< "<p>Count: " << ++count << "</p>"
				<< "<p>Host: " << req.getHost() << "</p>"
				<< "<p>Method: " << req.getMethod() << "</p>"
				<< "<p>URI: " << req.getURI() << "</p>"
				<< "<p>URI path: " << uri.getPath() << "</p>"
				<< "<p>getPathAndQuery: " << uri.getPathAndQuery() << "</p>"
				<< "<p>getFragment: " << uri.getFragment() << "</p>"
				<< "<p>getPathEtc: " << uri.getPathEtc() << "</p>"
				<< "<p>getQuery: " << uri.getQuery() << "</p>"
				<< "<textarea rows=20 cols=200>" << ssout.str() << "</textarea>";
		}
		
		out.flush();
		cout << "Rrequest flushed";

		cout << endl
			<< "Response sent for count=" << count
			<< " and URI=" << req.getURI() << endl;
	}

private:
	static int count;
};


//request handler factory
class MyRequestHandlerFactory : public HTTPRequestHandlerFactory
{
public:
	virtual HTTPRequestHandler* createRequestHandler(const HTTPServerRequest &) {
		return new MyRequestHandler;
	}
};

int MyRequestHandler::count = 0;

//main
int main(int argc, char** argv) {
	cout << "Start" << endl;
	
	
	HTTPServer s(new MyRequestHandlerFactory, ServerSocket(9090), new HTTPServerParams);
	s.start();

	int i = 0;
	cin >> i;
}