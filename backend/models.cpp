
#include "models.h"

// USER
User::User(const std::string &id,
           const std::string &name,
           const std::string &email)
    : id(id), name(name), email(email) {}

std::string User::getId() const
{
    return id;
}

std::string User::getName() const
{
    return name;
}

std::string User::getEmail() const
{
    return email;
}

// STUDENT
Student::Student(const std::string &id,
                 const std::string &name,
                 const std::string &email)
    : User(id, name, email) {}

std::string Student::getRole() const
{
    return "student";
}

std::string Student::getDashboard() const
{
    return "Student Dashboard";
}

// ADMIN
Admin::Admin(const std::string &id,
             const std::string &name,
             const std::string &email)
    : User(id, name, email) {}

std::string Admin::getRole() const
{
    return "admin";
}

std::string Admin::getDashboard() const
{
    return "Admin Dashboard";
}

// EVENT
Event::Event(const std::string &id,
             const std::string &title,
             const std::string &description,
             const std::string &venue)
    : id(id), title(title),
      description(description), venue(venue) {}

std::string Event::getId() const
{
    return id;
}

std::string Event::getTitle() const
{
    return title;
}

std::string Event::getDescription() const
{
    return description;
}

std::string Event::getVenue() const
{
    return venue;
}

void Event::updateDetails(
    const std::string &title,
    const std::string &description,
    const std::string &venue)
{
    this->title = title;
    this->description = description;
    this->venue = venue;
}

// REGISTRATION
Registration::Registration(
    const std::string &id,
    const std::string &studentId,
    const std::string &eventId)
    : id(id), studentId(studentId),
      eventId(eventId), status("pending") {}

std::string Registration::getStatus() const
{
    return status;
}

void Registration::approve()
{
    status = "approved";
}

void Registration::reject()
{
    status = "rejected";
}

void Registration::cancel()
{
    status = "cancelled";
}
