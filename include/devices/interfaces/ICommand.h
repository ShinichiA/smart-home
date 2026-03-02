#pragma once

#include <string>

namespace iot {

/**
 * @brief Command pattern interface for device actions with undo support.
 *
 * Design Patterns: Command
 */
class ICommand {
public:
    virtual ~ICommand() = default;

    virtual void execute() = 0;
    virtual void undo() = 0;

    [[nodiscard]] virtual std::string getDescription() const = 0;

protected:
    ICommand() = default;
    ICommand(const ICommand&) = default;
    ICommand& operator=(const ICommand&) = default;
    ICommand(ICommand&&) = default;
    ICommand& operator=(ICommand&&) = default;
};

} // namespace iot
