#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>

class Server {
public:
    Server(int port = 8080);
    ~Server();
    
    // Main server operations
    bool start();
    void run();
    
private:
    // Socket operations
    bool createSocket();
    bool bindSocket();
    bool listenForConnections();
    
    // Request handling
    std::string readHttpRequest(int clientSocket);
    bool parseRequest(const std::string& request, std::string& method, 
                     std::string& path, std::string& queryString);
    
    // Response handling
    void sendHttpResponse(int clientSocket, const std::string& content);
    
    // CGI operations
    std::string executePhpScript(const std::string& scriptPath, 
                                const std::string& queryString,
                                const std::string& postData);
    
    // Member variables
    int serverSocket;
    int port;
    static const int MAX_BUFFER_SIZE = 8192;
};

#endif // SERVER_HPP 