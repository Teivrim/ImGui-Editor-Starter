#pragma once
#include "../core/Types.h"
#include <string>

class Command {
public:
    virtual ~Command() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual std::string name() const = 0;
    virtual bool mergeWith(const Command& other) { return false; }
};

class LambdaCommand : public Command {
public:
    LambdaCommand(std::string name, std::function<void()> exec, std::function<void()> un)
        : m_name(std::move(name)), m_exec(std::move(exec)), m_undo(std::move(un)) {}

    void execute() override { m_exec(); }
    void undo() override { m_undo(); }
    std::string name() const override { return m_name; }

private:
    std::string m_name;
    std::function<void()> m_exec;
    std::function<void()> m_undo;
};

template<typename T>
class ValueCommand : public Command {
public:
    ValueCommand(std::string name, T* target, T oldValue, T newValue)
        : m_name(std::move(name)), m_target(target),
          m_oldValue(std::move(oldValue)), m_newValue(std::move(newValue)) {}

    void execute() override { *m_target = m_newValue; }
    void undo() override { *m_target = m_oldValue; }
    std::string name() const override { return m_name; }

private:
    std::string m_name;
    T* m_target;
    T m_oldValue;
    T m_newValue;
};
