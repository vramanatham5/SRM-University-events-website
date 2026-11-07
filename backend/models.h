
#ifndef MODELS_H
#define MODELS_H

#include <string>

// Abstract base class
class User
{
private:
    std::string id;
    std::string name;
    std::string email;

public:
    User(const std::string &id,
         const std::string &name,
         const std::string &email);

    virtual ~User() = default;

    std::string getId() const;
    std::string getName() const;
    std::string getEmail() const;

    virtual std::string getRole() const = 0;
    virtual std::string getDashboard() const = 0;
};

// Student class
class Student : public User
{
public:
    Student(const std::string &id,
            const std::string &name,
            const std::string &email);

    std::string getRole() const override;
    std::string getDashboard() const override;
};

// Admin class
class Admin : public User
{
public:
    Admin(const std::string &id,
          const std::string &name,
          const std::string &email);

    std::string getRole() const override;
    std::string getDashboard() const override;
};

// Event class
class Event
{
private:
    std::string id;
    std::string title;
    std::string description;
    std::string venue;

public:
    Event(const std::string &id,
          const std::string &title,
          const std::string &description,
          const std::string &venue);

    std::string getId() const;
    std::string getTitle() const;
    std::string getDescription() const;
    std::string getVenue() const;

    void updateDetails(
        const std::string &title,
        const std::string &description,
        const std::string &venue);
};

// Registration class
class Registration
{
private:
    std::string id;
    std::string studentId;
    std::string eventId;
    std::string status;

public:
    Registration(const std::string &id,
                 const std::string &studentId,
                 const std::string &eventId);

    std::string getStatus() const;

    void approve();
    void reject();
    void cancel();
};

#endif
