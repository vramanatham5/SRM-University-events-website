#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#ifdef _MSC_VER
#pragma comment(lib, "Ws2_32.lib")
#endif
using Socket = SOCKET;
static constexpr Socket invalid_socket = INVALID_SOCKET;
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
using Socket = int;
static constexpr Socket invalid_socket = -1;
#endif

namespace {

constexpr int default_port = 8080;
constexpr std::size_t max_request_size = 1024 * 1024;

int configured_port() {
  const char* environment_port = std::getenv("PORT");
  if (environment_port == nullptr || *environment_port == '\0') return default_port;

  errno = 0;
  char* end = nullptr;
  const long parsed_port = std::strtol(environment_port, &end, 10);
  if (errno != 0 || end == environment_port || *end != '\0' ||
      parsed_port < 1 || parsed_port > 65535) {
    std::cerr << "PORT must be a number between 1 and 65535.\n";
    return 0;
  }
  return static_cast<int>(parsed_port);
}

const char* registrations_file() {
  const char* path = std::getenv("REGISTRATIONS_FILE");
  return path != nullptr && *path != '\0' ? path : "registrations.csv";
}

const char* public_origin() {
  const char* origin = std::getenv("PUBLIC_ORIGIN");
  return origin != nullptr && *origin != '\0' ? origin : "";
}

void close_socket(Socket socket) {
#ifdef _WIN32
  closesocket(socket);
#else
  close(socket);
#endif
}

bool send_all(Socket socket, const std::string& data) {
  std::size_t sent = 0;
  while (sent < data.size()) {
#ifdef _WIN32
    const int result = send(socket, data.data() + sent, static_cast<int>(data.size() - sent), 0);
#else
    const auto result = send(socket, data.data() + sent, data.size() - sent, 0);
#endif
    if (result <= 0) return false;
    sent += static_cast<std::size_t>(result);
  }
  return true;
}

std::string lower(std::string value) {
  std::transform(value.begin(), value.end(), value.begin(), [](unsigned char character) {
    return static_cast<char>(std::tolower(character));
  });
  return value;
}

std::string trim(std::string value) {
  const auto first = value.find_first_not_of(" \t\r\n");
  if (first == std::string::npos) return {};
  const auto last = value.find_last_not_of(" \t\r\n");
  return value.substr(first, last - first + 1);
}

struct Request {
  std::string method;
  std::string path;
  std::string body;
  std::string origin;
};

bool read_request(Socket socket, Request& request) {
  std::string data;
  char buffer[4096];
  std::size_t header_end = std::string::npos;
  std::size_t content_length = 0;

  while (header_end == std::string::npos) {
#ifdef _WIN32
    const int received = recv(socket, buffer, sizeof(buffer), 0);
#else
    const auto received = recv(socket, buffer, sizeof(buffer), 0);
#endif
    if (received <= 0) return false;
    data.append(buffer, static_cast<std::size_t>(received));
    if (data.size() > max_request_size) return false;
    header_end = data.find("\r\n\r\n");
  }

  std::istringstream headers(data.substr(0, header_end));
  std::string request_line;
  if (!std::getline(headers, request_line)) return false;
  std::istringstream request_parts(request_line);
  if (!(request_parts >> request.method >> request.path)) return false;

  std::string line;
  while (std::getline(headers, line)) {
    const auto separator = line.find(':');
    if (separator == std::string::npos) continue;
    const std::string name = lower(trim(line.substr(0, separator)));
    if (name == "origin") {
      request.origin = trim(line.substr(separator + 1));
    } else if (name == "content-length") {
      try {
        const auto parsed = std::stoull(trim(line.substr(separator + 1)));
        if (parsed > max_request_size) return false;
        content_length = static_cast<std::size_t>(parsed);
      } catch (const std::exception&) {
        return false;
      }
    }
  }

  const std::size_t body_start = header_end + 4;
  while (data.size() - body_start < content_length) {
#ifdef _WIN32
    const int received = recv(socket, buffer, sizeof(buffer), 0);
#else
    const auto received = recv(socket, buffer, sizeof(buffer), 0);
#endif
    if (received <= 0) return false;
    data.append(buffer, static_cast<std::size_t>(received));
    if (data.size() > max_request_size) return false;
  }
  request.body = data.substr(body_start, content_length);
  return true;
}

std::string json_escape(const std::string& value) {
  std::string escaped;
  for (unsigned char character : value) {
    switch (character) {
      case '"': escaped += "\\\""; break;
      case '\\': escaped += "\\\\"; break;
      case '\b': escaped += "\\b"; break;
      case '\f': escaped += "\\f"; break;
      case '\n': escaped += "\\n"; break;
      case '\r': escaped += "\\r"; break;
      case '\t': escaped += "\\t"; break;
      default:
        if (character < 0x20) {
          const char hex[] = "0123456789abcdef";
          escaped += "\\u00";
          escaped += hex[character >> 4];
          escaped += hex[character & 0x0f];
        } else {
          escaped += static_cast<char>(character);
        }
    }
  }
  return escaped;
}

bool json_string(const std::string& json, const std::string& key, std::string& value) {
  const std::string needle = "\"" + key + "\"";
  const auto key_position = json.find(needle);
  if (key_position == std::string::npos) return false;
  auto position = json.find(':', key_position + needle.size());
  if (position == std::string::npos) return false;
  ++position;
  while (position < json.size() && std::isspace(static_cast<unsigned char>(json[position]))) ++position;
  if (position >= json.size() || json[position] != '"') return false;
  ++position;

  value.clear();
  while (position < json.size()) {
    const char character = json[position++];
    if (character == '"') return true;
    if (character == '\\') {
      if (position >= json.size()) return false;
      const char escaped = json[position++];
      switch (escaped) {
        case '"': value += '"'; break;
        case '\\': value += '\\'; break;
        case '/': value += '/'; break;
        case 'b': value += '\b'; break;
        case 'f': value += '\f'; break;
        case 'n': value += '\n'; break;
        case 'r': value += '\r'; break;
        case 't': value += '\t'; break;
        default: return false;
      }
    } else {
      if (static_cast<unsigned char>(character) < 0x20) return false;
      value += character;
    }
  }
  return false;
}

void respond(Socket socket, int status, const std::string& status_text,
             const std::string& content_type, const std::string& body,
             const std::string& allowed_origin = "") {
  std::ostringstream response;
  response << "HTTP/1.1 " << status << " " << status_text << "\r\n"
           << "Content-Type: " << content_type << "\r\n"
           << "Content-Length: " << body.size() << "\r\n"
           << "Connection: close\r\n"
           << "X-Content-Type-Options: nosniff\r\n";
  if (!allowed_origin.empty()) {
    response << "Access-Control-Allow-Origin: " << allowed_origin << "\r\n"
             << "Access-Control-Allow-Methods: POST, OPTIONS\r\n"
             << "Access-Control-Allow-Headers: Content-Type\r\n"
             << "Vary: Origin\r\n";
  }
  response << "Content-Security-Policy: default-src 'self' https://fonts.googleapis.com https://fonts.gstatic.com; style-src 'self' 'unsafe-inline' https://fonts.googleapis.com; font-src 'self' https://fonts.gstatic.com; script-src 'self'; connect-src 'self'; img-src 'self' data:\r\n"
           << "\r\n" << body;
  send_all(socket, response.str());
}

std::string matching_public_origin(const Request& request) {
  const char* configured_origin = public_origin();
  if (request.origin.empty() || *configured_origin == '\0' ||
      request.origin != configured_origin) {
    return {};
  }
  return request.origin;
}

void respond_json(Socket socket, int status, const std::string& status_text,
                  const std::string& body, const std::string& allowed_origin = "") {
  respond(socket, status, status_text, "application/json; charset=utf-8", body, allowed_origin);
}

std::string csv_escape(const std::string& value) {
  std::string escaped = "\"";
  for (char character : value) {
    if (character == '"') escaped += '"';
    escaped += character;
  }
  return escaped + '"';
}

bool valid_registration(const std::map<std::string, std::string>& fields, std::string& error) {
  const std::vector<std::string> required = {"name", "email", "studentId", "eventId"};
  for (const auto& key : required) {
    const auto field = fields.find(key);
    if (field == fields.end() || trim(field->second).empty()) {
      error = "Please complete all registration fields.";
      return false;
    }
  }
  const auto& name = fields.at("name");
  const auto& email = fields.at("email");
  const auto& student_id = fields.at("studentId");
  const auto& event_id = fields.at("eventId");
  if (name.size() > 100 || email.size() > 254 || student_id.size() > 40) {
    error = "One or more fields are too long.";
    return false;
  }
  const auto at = email.find('@');
  if (at == std::string::npos || at == 0 || email.find('.', at) == std::string::npos) {
    error = "Enter a valid email address.";
    return false;
  }
  const std::vector<std::string> event_ids = {
      "codefest-hackathon", "react-workshop", "debugging-contest", "ai-tech-talk"};
  if (std::find(event_ids.begin(), event_ids.end(), event_id) == event_ids.end()) {
    error = "That event is not available for registration.";
    return false;
  }
  return true;
}

void handle_registration(Socket socket, const Request& request) {
  if (request.method != "POST") {
    respond_json(socket, 405, "Method Not Allowed", "{\"error\":\"Use POST to register.\"}",
                 matching_public_origin(request));
    return;
  }
  std::map<std::string, std::string> fields;
  for (const auto& key : {"name", "email", "studentId", "eventId"}) {
    if (!json_string(request.body, key, fields[key])) {
      respond_json(socket, 400, "Bad Request", "{\"error\":\"Registration data is invalid.\"}",
                   matching_public_origin(request));
      return;
    }
  }
  std::string error;
  if (!valid_registration(fields, error)) {
    respond_json(socket, 400, "Bad Request", "{\"error\":\"" + json_escape(error) + "\"}",
                 matching_public_origin(request));
    return;
  }

  std::ofstream registrations(registrations_file(), std::ios::app);
  if (!registrations) {
    std::cerr << "Unable to open " << registrations_file() << " for writing.\n";
    respond_json(socket, 500, "Internal Server Error", "{\"error\":\"We couldn't save your registration. Please try again.\"}",
                 matching_public_origin(request));
    return;
  }
  registrations << csv_escape(fields.at("name")) << ','
                << csv_escape(fields.at("email")) << ','
                << csv_escape(fields.at("studentId")) << ','
                << csv_escape(fields.at("eventId")) << '\n';
  registrations.close();
  if (!registrations) {
    std::cerr << "Unable to finish writing registrations.csv.\n";
    respond_json(socket, 500, "Internal Server Error", "{\"error\":\"We couldn't save your registration. Please try again.\"}",
                 matching_public_origin(request));
    return;
  }
  respond_json(socket, 201, "Created", "{\"message\":\"Registration saved.\"}",
               matching_public_origin(request));
}

void serve_file(Socket socket, const std::string& path) {
  const std::map<std::string, std::string> allowed_files = {
      {"/", "index.html"},
      {"/index.html", "index.html"},
      {"/styles.css", "styles.css"},
      {"/app.js", "app.js"},
      {"/config.json", "config.json"}};
  const auto file = allowed_files.find(path);
  if (file == allowed_files.end()) {
    respond(socket, 404, "Not Found", "text/plain; charset=utf-8", "Not found");
    return;
  }

  std::ifstream input(file->second, std::ios::binary);
  if (!input) {
    std::cerr << "Unable to open " << file->second << ". Run the server from the project directory.\n";
    respond(socket, 500, "Internal Server Error", "text/plain; charset=utf-8", "Unable to open website file");
    return;
  }
  std::ostringstream content;
  content << input.rdbuf();
  const std::string type = file->second == "index.html" ? "text/html; charset=utf-8"
      : file->second == "styles.css" ? "text/css; charset=utf-8"
      : file->second == "config.json" ? "application/json; charset=utf-8"
      : "text/javascript; charset=utf-8";
  respond(socket, 200, "OK", type, content.str());
}

void handle_client(Socket client) {
  Request request;
  if (!read_request(client, request)) {
    respond_json(client, 400, "Bad Request", "{\"error\":\"Invalid HTTP request.\"}");
    close_socket(client);
    return;
  }
  if (request.path == "/api/register") {
    if (request.method == "OPTIONS") {
      const std::string allowed_origin = matching_public_origin(request);
      if (allowed_origin.empty()) {
        respond_json(client, 403, "Forbidden", "{\"error\":\"This website origin is not allowed.\"}");
      } else {
        respond(client, 204, "No Content", "text/plain; charset=utf-8", "", allowed_origin);
      }
    } else {
      handle_registration(client, request);
    }
  } else if (request.method != "GET" && request.method != "HEAD") {
    respond(client, 405, "Method Not Allowed", "text/plain; charset=utf-8", "Method not allowed");
  } else {
    serve_file(client, request.path);
  }
  close_socket(client);
}

}  // namespace

int main() {
  const int port = configured_port();
  if (port == 0) return 1;

#ifdef _WIN32
  WSADATA winsock_data;
  if (WSAStartup(MAKEWORD(2, 2), &winsock_data) != 0) {
    std::cerr << "Unable to initialize Winsock.\n";
    return 1;
  }
#endif

  Socket server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (server == invalid_socket) {
    std::cerr << "Unable to create server socket.\n";
#ifdef _WIN32
    WSACleanup();
#endif
    return 1;
  }

  int reuse_address = 1;
  setsockopt(server, SOL_SOCKET, SO_REUSEADDR,
#ifdef _WIN32
             reinterpret_cast<const char*>(&reuse_address),
#else
             &reuse_address,
#endif
             sizeof(reuse_address));

  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_ANY);
  address.sin_port = htons(port);
  if (bind(server, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == -1) {
    std::cerr << "Unable to bind port " << port << ". Is another server already using it?\n";
    close_socket(server);
#ifdef _WIN32
    WSACleanup();
#endif
    return 1;
  }
  if (listen(server, 16) == -1) {
    std::cerr << "Unable to listen on port " << port << ".\n";
    close_socket(server);
#ifdef _WIN32
    WSACleanup();
#endif
    return 1;
  }

  std::cout << "SRM University coding events are running at http://localhost:" << port << "\n";
  while (true) {
    Socket client = accept(server, nullptr, nullptr);
    if (client == invalid_socket) {
      std::cerr << "Unable to accept an incoming connection.\n";
      continue;
    }
    handle_client(client);
  }

  close_socket(server);
#ifdef _WIN32
  WSACleanup();
#endif
  return 0;
}
