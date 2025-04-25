#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Interface/ISystem.h"


/// @brief Handles the registration, dependency resolution, and ordered lifecycle (Init/Shutdown) of subsystems.
class SystemHandler
{
public:
    /// @brief Registers a subsystem instance by name.
    /// @param name The unique name of the subsystem.
    /// @param instance A pointer to the subsystem instance (owned externally).
	void Register(const std::string& name, ISystem* instance);

    /// @brief Clears all registered subsystems and their dependencies.
    void Clear();

    /// @brief Initializes all subsystems in topologically sorted order.
    bool BuildAll(SweetLoader& sweetLoader);

    /// @brief Shuts down all subsystems in reverse of initialization order.
    bool ShutdownAll();

    /// @brief Adds one or more dependencies for a given subsystem.
    /// @tparam Args Variadic list of subsystem names this system depends on.
    /// @param system The name of the system being configured.
    /// @param deps Names of systems this one depends on.
    template<typename... Args>
    void AddDependency(const std::string& system, const Args&... deps);

    void WaitAll();

private:
    /// @brief Performs topological sort on the dependency graph.
    /// @return Sorted list of subsystem names in initialization order.
    std::vector<std::string> TopologicalSort();

    /// @brief Recursive DFS utility for topological sort and cycle detection.
    /// @param node Current node to visit.
    /// @param visited Set of already visited nodes.
    /// @param recursionStack Stack to detect cycles.
    /// @param sorted Output vector of sorted subsystem names.
    void DFS(const std::string& node,
        std::unordered_set<std::string>& visited,
        std::unordered_set<std::string>& recursionStack,
        std::vector<std::string>& sorted);

private:
    /// Map of subsystem names to their instance pointers.
    std::unordered_map<std::string, ISystem*> m_registry;

    /// Map of subsystem names to their list of dependency names.
    std::unordered_map<std::string, std::vector<std::string>> m_dependencies;

    /// Ordered list of all registered system names (preserves insertion order).
    std::vector<std::string> m_systemNames;

    /// Final topologically sorted order (cached after BuildAll).
    std::vector<std::string> m_initOrder;
};

// Template implementation for adding multiple dependencies using fold expression.
template <typename ... Args>
void SystemHandler::AddDependency(const std::string& system, const Args&... deps)
{
    (m_dependencies[system].push_back(deps), ...);
}
