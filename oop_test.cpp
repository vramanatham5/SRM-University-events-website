
#include "backend/models.h"
#include <iostream>

int main()
{
    Student student("S101", "Student One", "student@example.com");
    Admin admin("A101", "Admin One", "admin@example.com");

    User *user1 = &student;
    User *user2 = &admin;

    std::cout << user1->getRole() << " - "
              << user1->getDashboard() << '\n';

    std::cout << user2->getRole() << " - "
              << user2->getDashboard() << '\n';

    Event event("E101", "CodeFest",
                "Coding competition", "SRM Campus");

    std::cout << "Event: " << event.getTitle() << '\n';

    Registration registration("R101", "S101", "E101");

    registration.approve();

    std::cout << "Registration: "
              << registration.getStatus() << '\n';

    return 0;
}
