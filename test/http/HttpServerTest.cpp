#include <map>
#include <string>
#include <fstream>
#include <iostream>

#include "HttpServer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

std::string image;
bool benchmark = true;

std::string GetFile();
void OnRequest(const CHttpRequest& req, CHttpResponse& resp) {
    //std::cout << "Headers " << req.GetMethodString() << " " << req.GetPath() << std::endl;
    if (!benchmark) {
        const std::map<std::string, std::string>& headers = req.GetHeaders();
        for (const auto& header : headers) {
            std::cout << header.first << ": " << header.second << std::endl;
        }
    }

    if (req.GetPath() == "/") {
        resp.SetStatusCode(k200Ok);
        resp.SetStatusMessage("OK");
        resp.SetContentType("text/html");
        resp.AddHeader("Server", "CppNet");
        CHttpServer::_time_tool.Now();
        std::string now = std::to_string(CHttpServer::_time_tool.GetMsec());
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
    std::cout << "[DisConnectionFunc]" << std::endl;
}

int main() {
    cppnet::CCppNet net;
    net.Init(2);

    CHttpServer server;
    server.SetHttpCallback(OnRequest);

    net.SetAcceptCallback(std::bind(&CHttpServer::OnConnection, &server, std::placeholders::_1, std::placeholders::_2));
    net.SetWriteCallback(std::bind(&CHttpServer::OnMessageSend, &server, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    net.SetReadCallback(std::bind(&CHttpServer::OnMessage, &server, std::placeholders::_1, std::placeholders::_2, 
                                              std::placeholders::_3, std::placeholders::_4));
    net.SetDisconnectionCallback(DisConnectionFunc);

    net.ListenAndAccept("0.0.0.0", 8921);

    net.Join();
}
