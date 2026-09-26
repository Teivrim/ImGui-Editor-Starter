#pragma once
#include "../core/Types.h"
#include "Command.h"
#include <vector>
#include <memory>

class UndoStack {
public:
    explicit UndoStack(u32 maxCommands = 100);

    void push(std::unique_ptr<Command> command);
    bool undo();
    bool redo();

    void undoTo(i32 targetIndex);
    i32 currentIndex() const { return m_currentIndex; }

    bool canUndo() const { return m_currentIndex > 0; }
    bool canRedo() const { return m_currentIndex < m_commands.size(); }
    u32 commandCount() const { return (u32)m_commands.size(); }

    std::string undoName() const;
    std::string redoName() const;
    std::string commandName(u32 index) const;

    void clear();
    void setMaxCommands(u32 max) { m_maxCommands = max; }
    u32 maxCommands() const { return m_maxCommands; }

    void beginBatch(const std::string& name);
    void endBatch();
    bool isInBatch() const { return m_batchDepth > 0; }

private:
    std::vector<std::unique_ptr<Command>> m_commands;
    i32 m_currentIndex = 0;
    u32 m_maxCommands = 100;
    u32 m_batchDepth = 0;
    std::vector<std::unique_ptr<Command>> m_batchCommands;
    std::string m_batchName;
};
