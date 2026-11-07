
#include "backend/supabase_client.h"

#include <iostream>

int main()
{
    SupabaseClient supabase;

    if (!supabase.isConfigured())
    {
        std::cout << "Supabase configuration not found.\n";
        return 1;
    }

    std::cout << "Supabase configuration loaded.\n";

    SupabaseResponse response = supabase.request(
        "GET",
        "/rest/v1/portal_events?select=id,title");

    std::cout << "HTTP Status: "
              << response.status_code << '\n';

    if (!response.error.empty())
    {
        std::cout << "Error: " << response.error << '\n';
        return 1;
    }

    std::cout << "Response: " << response.body << '\n';

    if (response.status_code == 200)
    {
        std::cout << "Supabase connection successful!\n";
        return 0;
    }

    std::cout << "Supabase returned an error.\n";
    return 1;
}
