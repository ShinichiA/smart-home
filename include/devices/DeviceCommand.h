#pragma once

#include "devices/interfaces/ICommand.h"
#include <memory>
#include <vector>
#include <functional>
#include <stack>

namespace iot {

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
