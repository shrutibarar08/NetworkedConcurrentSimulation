#include "SystemHandler.h"

#include <stdexcept>

void SystemHandler::Register(const std::string& name, ISystem* instance)
{
    // Register system if not already present
    if (!instance || m_registry.contains(name)) return;

    m_registry[name] = instance;
    m_dependencies[name] = {};
    m_systemNames.push_back(name);
}

void SystemHandler::Clear()
{
    // Clears all registered systems and dependency metadata
    m_registry.clear();
    m_dependencies.clear();
    m_systemNames.clear();
}

bool SystemHandler::BuildAll(SweetLoader& sweetLoader)
{
    // Compute topological order and initialize each system in order
    m_initOrder = TopologicalSort();
    bool error = false;

    for (const auto& name : m_initOrder)
    {
        if (auto* system = m_registry.at(name))
        {
            if (system->Init())
            {
                system->Build(sweetLoader[name]);
            }else
            {
                error = true;
            }
        }
    }

    return !error;
}

bool SystemHandler::ShutdownAll()
{
    bool error = false;
    // Shutdown systems in reverse order of initialization
    for (auto it = m_initOrder.rbegin(); it != m_initOrder.rend(); ++it)
    {
        if (auto* system = m_registry.at(*it))
        {
            if (!system->Shutdown()) error = true;
        }
    }
    return !error;
}

void SystemHandler::WaitStart()
{
    std::vector<HANDLE> handles;
    for (auto it = m_initOrder.rbegin(); it != m_initOrder.rend(); ++it)
    {
        if (auto* system = m_registry.at(*it))
        {
            if (HANDLE handle = system->GetInitializedEventHandle())
            {
                handles.push_back(handle);
            }
        }
    }
    DWORD objectCount = static_cast<DWORD>(handles.size());
    WaitForMultipleObjects(objectCount,
        handles.data(), TRUE,
        INFINITE);
}

void SystemHandler::WaitFinish()
{
    std::vector<HANDLE> handles;
    for (auto it = m_initOrder.rbegin(); it != m_initOrder.rend(); ++it)
    {
        if (auto* system = m_registry.at(*it))
        {
            if (HANDLE handle = system->GetThreadHandle())
            {
                handles.push_back(handle);
            }
        }
    }

    DWORD size = static_cast<DWORD>(handles.size());
    WaitForMultipleObjects(size,
        handles.data(), TRUE,
        INFINITE);
}

std::vector<std::string> SystemHandler::TopologicalSort()
{
    std::unordered_set<std::string> visited;
    std::unordered_set<std::string> recursionStack;
    std::vector<std::string> sorted;

    // Visit all registered systems
    for (const auto& name : m_systemNames)
    {
        if (!visited.contains(name))
            DFS(name, visited, recursionStack, sorted);
    }

    return sorted;
}

void SystemHandler::DFS(const std::string& node, std::unordered_set<std::string>& visited, std::unordered_set<std::string>& recursionStack, std::vector<std::string>& sorted)
{
    // Cycle detection
    if (recursionStack.contains(node))
        throw std::runtime_error("Circular dependency detected at: " + node);

    if (visited.contains(node)) return;

    visited.insert(node);
    recursionStack.insert(node);

    // Recursively resolve dependencies first
    for (const auto& dep : m_dependencies[node])
    {
        DFS(dep, visited, recursionStack, sorted);
    }

    recursionStack.erase(node);
    sorted.push_back(node);
}
