#include <string>
#include <fstream>
#include <iostream>
#include <unordered_map>

#include "http_server.h"
#include "http_request.h"
#include "http_response.h"

#include "foundation/util/time.h"

std::string image;
bool benchmark = true;

std::string GetFile();
void OnRequest(const HttpRequest& req, HttpResponse& resp) {
    //std::cout << "Headers " << req.GetMethodString() << " " << req.GetPath() << std::endl;
    if (!benchmark) {
        const std::unordered_map<std::string, std::string>& headers = req.GetHeaders();
        for (const auto& header : headers) {
            std::cout << header.first << ": " << header.second << std::endl;
        }
    }

    if (req.GetPath() == "/") {
        resp.SetStatusCode(k200Ok);
        resp.SetStatusMessage("OK");
        resp.SetContentType("text/html");
        resp.AddHeader("Server", "CppNet");
        std::string now = fdan::GetFormatTime();
        resp.SetBody("<html><head><title>This is title</title></head>"
            "<body><h1>Hello</h1>Now is " + now +
            "</body></html>");

    } else if (req.GetPath() == "/favicon.ico") {
        resp.SetStatusCode(k200Ok);
        resp.SetStatusMessage("OK");
        resp.SetContentType("image/png");
        if (image.empty()) {
            image = GetFile();
        }
        resp.SetBody(image);

    } else if (req.GetPath() == "/hello") {
        resp.SetStatusCode(k200Ok);
        resp.SetStatusMessage("OK");
        resp.SetContentType("text/plain");
        resp.AddHeader("Server", "CppNet");
        resp.SetBody("hello, world!\n");

    } else {
        resp.SetStatusCode(k404NotFound);
        resp.SetStatusMessage("Not Found");
        resp.SetCloseConnection(true);
    }
}

std::string GetFile() {
    std::ifstream t("logo.png");
    std::string str((std::istreambuf_iterator<char>(t)),
        std::istreambuf_iterator<char>());
    return std::move(str);
}

void DisConnectionFunc(const cppnet::Handle& , uint32_t ) {
    //std::cout << "[DisConnectionFunc]" << std::endl;
}

int main() {
    cppnet::CppNet net;
    net.Init(2);

    HttpServer server;
    server.SetHttpCallback(OnRequest);

    net.SetAcceptCallback(std::bind(&HttpServer::OnConnection, &server, std::placeholders::_1, std::placeholders::_2));
    net.SetWriteCallback(std::bind(&HttpServer::OnMessageSend, &server, std::placeholders::_1, std::placeholders::_2));
    net.SetReadCallback(std::bind(&HttpServer::OnMessage, &server, std::placeholders::_1, std::placeholders::_2, 
                                              std::placeholders::_3));
    net.SetDisconnectionCallback(std::bind(&HttpServer::OnDisConnection, &server, std::placeholders::_1, std::placeholders::_2));

    net.ListenAndAccept("0.0.0.0", 8921);

    net.Join();
}
