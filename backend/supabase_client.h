
#ifndef SUPABASE_CLIENT_H
#define SUPABASE_CLIENT_H

#include <string>

struct SupabaseResponse
{
    long status_code = 0;
    std::string body;
    std::string error;
};

class SupabaseClient
{
private:
    std::string project_url;
    std::string publishable_key;

public:
    SupabaseClient();

    bool isConfigured() const;

    SupabaseResponse request(
        const std::string &method,
        const std::string &endpoint,
        const std::string &body = "",
        const std::string &access_token = "");
};

#endif
