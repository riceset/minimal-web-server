#include "server.hpp"

template<typename T>
std::string toString(T value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

Server::Server(int port) : serverSocket(-1), port(port) {}

Server::~Server() {
    if (serverSocket != -1) {
        close(serverSocket);
    }
}

bool Server::start() {
    if (!createSocket()) return false;
    if (!bindSocket()) return false;
    if (!listenForConnections()) return false;
    
    std::cout << "Server listening on port " << port << "..." << std::endl;
    return true;
}

bool Server::createSocket() {
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error creating socket: " << strerror(errno) << std::endl;
        return false;
    }
    
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Error setting socket options: " << strerror(errno) << std::endl;
        return false;
    }
    
    return true;
}

bool Server::bindSocket() {
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);
    
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Error binding socket: " << strerror(errno) << std::endl;
        return false;
    }
    
    return true;
}

bool Server::listenForConnections() {
    if (listen(serverSocket, 5) < 0) {
        std::cerr << "Error listening on socket: " << strerror(errno) << std::endl;
        return false;
    }
    return true;
}

void Server::run() {
    while (true) {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        
        std::cout << "Waiting for connection..." << std::endl;
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
        if (clientSocket < 0) {
            std::cerr << "Error accepting connection: " << strerror(errno) << std::endl;
            continue;
        }
        
        std::cout << "Accepted connection from " << inet_ntoa(clientAddr.sin_addr) << std::endl;
        
        // Handle the request
        std::string request = readHttpRequest(clientSocket);
        std::string method, path, queryString;
        
        if (!parseRequest(request, method, path, queryString)) {
            std::cerr << "Error parsing request" << std::endl;
            close(clientSocket);
            continue;
        }
        
        // Process the request
        if (path.find(".php") != std::string::npos) {
            std::string scriptPath = "." + path;
            std::cout << "Processing PHP script: " << scriptPath << std::endl;
            std::string output = executePhpScript(scriptPath, queryString, "");
            sendHttpResponse(clientSocket, output);
        } else {
            sendHttpResponse(clientSocket, "Only PHP scripts are supported");
        }
        
        close(clientSocket);
        std::cout << "Connection closed" << std::endl;
    }
}

std::string Server::readHttpRequest(int clientSocket) {
    char buffer[MAX_BUFFER_SIZE];
    std::string request;
    int bytesRead;
    
    std::cout << "Reading HTTP request..." << std::endl;
    while ((bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytesRead] = '\0';
        request += buffer;
        std::cout << "Read " << bytesRead << " bytes" << std::endl;
        
        if (request.find("\r\n\r\n") != std::string::npos) {
            std::cout << "Found end of request" << std::endl;
            break;
        }
    }
    std::cout << "Request content:\n" << request << std::endl;
    return request;
}

bool Server::parseRequest(const std::string& request, std::string& method, 
                         std::string& path, std::string& queryString) {
    std::cout << "Parsing request..." << std::endl;
    
    size_t firstLineEnd = request.find("\r\n");
    if (firstLineEnd == std::string::npos) {
        std::cout << "No first line found" << std::endl;
        return false;
    }
    
    std::string firstLine = request.substr(0, firstLineEnd);
    std::cout << "First line: " << firstLine << std::endl;
    
    size_t methodEnd = firstLine.find(" ");
    if (methodEnd == std::string::npos) {
        std::cout << "No method found" << std::endl;
        return false;
    }
    method = firstLine.substr(0, methodEnd);
    std::cout << "Method: " << method << std::endl;
    
    size_t pathStart = methodEnd + 1;
    size_t pathEnd = firstLine.find(" ", pathStart);
    if (pathEnd == std::string::npos) {
        std::cout << "No path end found" << std::endl;
        return false;
    }
    
    std::string fullPath = firstLine.substr(pathStart, pathEnd - pathStart);
    std::cout << "Full path: " << fullPath << std::endl;
    
    size_t queryStart = fullPath.find("?");
    if (queryStart == std::string::npos) {
        path = fullPath;
        queryString = "";
    } else {
        path = fullPath.substr(0, queryStart);
        queryString = fullPath.substr(queryStart + 1);
    }
    
    std::cout << "Path: " << path << std::endl;
    std::cout << "Query string: " << queryString << std::endl;
    
    return true;
}

void Server::sendHttpResponse(int clientSocket, const std::string& content) {
    std::cout << "Sending response of length: " << content.length() << std::endl;
    std::string response = "HTTP/1.1 200 OK\r\n"
                          "Content-Type: text/html\r\n"
                          "Content-Length: " + toString(content.length()) + "\r\n"
                          "\r\n" + content;
    
    int bytesWritten = write(clientSocket, response.c_str(), response.length());
    std::cout << "Wrote " << bytesWritten << " bytes to client" << std::endl;
}

std::string Server::executePhpScript(const std::string& scriptPath, 
                                   const std::string& queryString,
                                   const std::string& postData) {
    std::cout << "Executing PHP script: " << scriptPath << std::endl;
    
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        std::cerr << "Error creating pipe" << std::endl;
        return "Error creating pipe";
    }
    
    pid_t pid = fork();
    if (pid == -1) {
        std::cerr << "Error forking process" << std::endl;
        return "Error forking process";
    }
    
    if (pid == 0) {  // Child process
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        
        // Prepare environment variables
        std::string envMethod = std::string("REQUEST_METHOD=") + (postData.empty() ? "GET" : "POST");
        std::string envQuery = std::string("QUERY_STRING=") + queryString;
        std::string envScript = std::string("SCRIPT_NAME=") + scriptPath;
        std::string envLength = std::string("CONTENT_LENGTH=") + (postData.empty() ? "0" : toString(postData.length()));
        
        // Create environment array
        char* env[] = {
            const_cast<char*>(envMethod.c_str()),
            const_cast<char*>(envQuery.c_str()),
            const_cast<char*>(envScript.c_str()),
            const_cast<char*>(envLength.c_str()),
            NULL
        };
        
        // Prepare arguments for execve
        char* args[] = {
            const_cast<char*>("/usr/local/bin/php"),
            const_cast<char*>(scriptPath.c_str()),
            NULL
        };
        
        std::cout << "Executing PHP with command: /usr/local/bin/php " << scriptPath << std::endl;
        execve("/usr/local/bin/php", args, env);
        std::cerr << "Error executing PHP: " << strerror(errno) << std::endl;
        std::exit(1);
    } else {  // Parent process
        close(pipefd[1]);
        
        char buffer[MAX_BUFFER_SIZE];
        std::string output;
        int bytesRead;
        
        while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytesRead] = '\0';
            output += buffer;
            std::cout << "Read " << bytesRead << " bytes from PHP output" << std::endl;
        }
        
        close(pipefd[0]);
        waitpid(pid, NULL, 0);
        
        return output;
    }
} 