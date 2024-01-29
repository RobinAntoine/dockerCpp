#include <iostream>
#include <string>
#include "SocketClient.hpp"


int main(int argc, char *argv[]){

    std::string url = "http://localhost/containers/json";
    SocketClient* client = SocketClient::getInstance("/var/run/docker.sock");
    std::string response;

    long response_code = client->get(url, response);

    std::cout   << "Response code: " << response_code << std::endl
                << "Response: " << response << std::endl;

    response.clear();
    long response_code2 = client->post("http://localhost/networks/create", "{\"name\":\"data\"}", response);

    std::cout   << "Response code: " << response_code << std::endl
                << "Response: " << response << std::endl;

    response.clear();
    long response_code3 = client->del("http://localhost/networks/data", response);

    std::cout   << "Response code: " << response_code << std::endl
                << "Response: " << response << std::endl;

    response.clear();
    return 0;
}