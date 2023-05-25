//
// Created by juperez on 5/25/23.
//

#pragma once

#include <filesystem>
#include <vector>

using std::filesystem::path;

class Resolver {
public:
    typedef std::vector<path>::iterator iterator;
    typedef std::vector<path>::const_iterator const_iterator;

    Resolver() {
        m_paths.push_back(std::filesystem::current_path());
    }

    size_t size() const { return m_paths.size(); }

    iterator begin() { return m_paths.begin(); }
    iterator end()   { return m_paths.end(); }

    const_iterator begin() const { return m_paths.begin(); }
    const_iterator end()   const { return m_paths.end(); }

    void erase(iterator it) { m_paths.erase(it); }

    void prepend(const path &path) { m_paths.insert(m_paths.begin(), path); }
    void append(const path &path) { m_paths.push_back(path); }
    const path &operator[](size_t index) const { return m_paths[index]; }
    path &operator[](size_t index) { return m_paths[index]; }

    path resolve(const path &value) const {
        for (const_iterator it = m_paths.begin(); it != m_paths.end(); ++it) {
            path combined = *it / value;
            if (exists(combined))
                return combined;
        }
        return value;
    }

    friend std::ostream &operator<<(std::ostream &os, const Resolver &r) {
        os << "resolver[" << std::endl;
        for (size_t i = 0; i < r.m_paths.size(); ++i) {
            os << "  \"" << r.m_paths[i] << "\"";
            if (i + 1 < r.m_paths.size())
                os << ",";
            os << std::endl;
        }
        os << "]";
        return os;
    }

private:
    std::vector<path> m_paths;
};