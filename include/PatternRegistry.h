#pragma once

#include "Pattern.h"

class PatternRegistry
{
    mutable std::unordered_map<std::string, std::shared_ptr<Pattern>> instances_;

    static PatternRegistry* instance_;
    PatternRegistry() = default;

    static PatternRegistry& getInstance()
    {
        if (!instance_) instance_ = new PatternRegistry();
        return *instance_;
    }

public:
    template <typename T, typename... Args> requires IPattern<T>
    static void add(Args&&... args)
    {
        auto pattern = std::make_shared<T>(std::forward<Args>(args)...);
        const std::string& id = pattern->getId();

        if (has(id))
        {
            Serial.printf("WARNING: Pattern with ID '%s' already exists! Overwriting it...", id.c_str());
        }

        getInstance().instances_[id] = pattern;
    }

    static std::shared_ptr<Pattern> get(const std::string& id)
    {
        return getInstance().instances_.at(id);
    }

    static bool has(const std::string& id)
    {
        return getInstance().instances_.contains(id);
    }
};

PatternRegistry* PatternRegistry::instance_;
