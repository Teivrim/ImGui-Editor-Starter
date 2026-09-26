#include "UndoStack.h"
#include <algorithm>

class BatchCommand : public Command {
public:
    explicit BatchCommand(std::string name, std::vector<std::unique_ptr<Command>> cmds)
        : m_name(std::move(name)), m_commands(std::move(cmds)) {}

    void execute() override {
        for (auto& cmd : m_commands) cmd->execute();
    }

    void undo() override {
        for (auto it = m_commands.rbegin(); it != m_commands.rend(); ++it)
            (*it)->undo();
    }

    std::string name() const override { return m_name; }

private:
    std::string m_name;
    std::vector<std::unique_ptr<Command>> m_commands;
};

UndoStack::UndoStack(u32 maxCommands) : m_maxCommands(maxCommands) {}

void UndoStack::push(std::unique_ptr<Command> command) {
    if (m_batchDepth > 0) {
        m_batchCommands.push_back(std::move(command));
        return;
    }

    m_commands.erase(m_commands.begin() + m_currentIndex, m_commands.end());

    if (!m_commands.empty() && m_commands.back()->mergeWith(*command))
        return;

    m_commands.push_back(std::move(command));
    m_currentIndex = (i32)m_commands.size();

    if (m_commands.size() > m_maxCommands) {
        m_commands.erase(m_commands.begin());
        m_currentIndex--;
    }
}

bool UndoStack::undo() {
    if (!canUndo()) return false;
    m_currentIndex--;
    m_commands[m_currentIndex]->undo();
    return true;
}

bool UndoStack::redo() {
    if (!canRedo()) return false;
    m_commands[m_currentIndex]->execute();
    m_currentIndex++;
    return true;
}

void UndoStack::undoTo(i32 targetIndex) {
    targetIndex = std::max(0, std::min((i32)m_commands.size(), targetIndex));
    while (m_currentIndex > targetIndex) undo();
    while (m_currentIndex < targetIndex) redo();
}

std::string UndoStack::undoName() const {
    if (canUndo())
        return m_commands[m_currentIndex - 1]->name();
    return {};
}

std::string UndoStack::redoName() const {
    if (canRedo())
        return m_commands[m_currentIndex]->name();
    return {};
}

std::string UndoStack::commandName(u32 index) const {
    if (index < m_commands.size())
        return m_commands[index]->name();
    return {};
}

void UndoStack::clear() {
    m_commands.clear();
    m_currentIndex = 0;
}

void UndoStack::beginBatch(const std::string& name) {
    if (m_batchDepth == 0) {
        m_batchCommands.clear();
        m_batchName = name;
    }
    m_batchDepth++;
}

void UndoStack::endBatch() {
    if (m_batchDepth == 0) return;
    m_batchDepth--;
    if (m_batchDepth == 0 && !m_batchCommands.empty()) {
        push(std::make_unique<BatchCommand>(m_batchName, std::move(m_batchCommands)));
        m_batchCommands.clear();
    }
}
