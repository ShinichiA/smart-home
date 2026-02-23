#pragma once

#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <stack>

namespace iot {

/**
 * @brief Command pattern for device actions with undo/redo support.
 *
 * Design Patterns: Command
 * Modern C++: std::function, std::unique_ptr, std::stack,
 *             move semantics, lambda-friendly
 */
class ICommand {
public:
    virtual ~ICommand() = default;

    virtual void execute() = 0;
    virtual void undo() = 0;

    [[nodiscard]] virtual std::string getDescription() const = 0;
};

/**
 * @brief Generic command using lambdas for execute/undo actions.
 */
class LambdaCommand : public ICommand {
public:
    LambdaCommand(std::string description,
                  std::function<void()> execFunc,
                  std::function<void()> undoFunc);

    void execute() override;
    void undo() override;
    [[nodiscard]] std::string getDescription() const override;

private:
    std::string description_;
    std::function<void()> execFunc_;
    std::function<void()> undoFunc_;
};

/**
 * @brief Command invoker with history and undo/redo stacks.
 */
class CommandInvoker {
public:
    CommandInvoker() = default;

    void executeCommand(std::unique_ptr<ICommand> cmd);
    void undoLast();
    void redoLast();

    [[nodiscard]] size_t historySize() const;
    [[nodiscard]] std::vector<std::string> getHistory() const;

    void clearHistory();

private:
    std::vector<std::unique_ptr<ICommand>> history_;
    std::stack<std::unique_ptr<ICommand>>  redoStack_;
};

} // namespace iot
