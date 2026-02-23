#include "devices/DeviceCommand.h"
#include "core/Logger.h"

namespace iot {

// ── LambdaCommand ────────────────────────────────────────────────────

LambdaCommand::LambdaCommand(std::string description,
                              std::function<void()> execFunc,
                              std::function<void()> undoFunc)
    : description_(std::move(description))
    , execFunc_(std::move(execFunc))
    , undoFunc_(std::move(undoFunc))
{}

void LambdaCommand::execute() {
    Logger::getInstance().info(
        "Execute: " + description_, "Command");
    execFunc_();
}

void LambdaCommand::undo() {
    Logger::getInstance().info(
        "Undo: " + description_, "Command");
    undoFunc_();
}

std::string LambdaCommand::getDescription() const {
    return description_;
}

// ── CommandInvoker ───────────────────────────────────────────────────

void CommandInvoker::executeCommand(std::unique_ptr<ICommand> cmd) {
    cmd->execute();
    history_.push_back(std::move(cmd));
    // Clear redo stack when new command is executed
    while (!redoStack_.empty()) redoStack_.pop();
}

void CommandInvoker::undoLast() {
    if (history_.empty()) {
        Logger::getInstance().warning(
            "Nothing to undo", "CommandInvoker");
        return;
    }

    auto cmd = std::move(history_.back());
    history_.pop_back();
    cmd->undo();
    redoStack_.push(std::move(cmd));
}

void CommandInvoker::redoLast() {
    if (redoStack_.empty()) {
        Logger::getInstance().warning(
            "Nothing to redo", "CommandInvoker");
        return;
    }

    auto cmd = std::move(redoStack_.top());
    redoStack_.pop();
    cmd->execute();
    history_.push_back(std::move(cmd));
}

size_t CommandInvoker::historySize() const {
    return history_.size();
}

std::vector<std::string> CommandInvoker::getHistory() const {
    std::vector<std::string> result;
    result.reserve(history_.size());
    for (const auto& cmd : history_) {
        result.push_back(cmd->getDescription());
    }
    return result;
}

void CommandInvoker::clearHistory() {
    history_.clear();
    while (!redoStack_.empty()) redoStack_.pop();
}

} // namespace iot
