#pragma once

#include "Pattern.h"

class Registry
{
    mutable std::unordered_map<std::string, std::shared_ptr<Pattern>> instances_;

    static Registry *instance_;
    Registry() = default;

    static Registry &getInstance()
    {
        if (!instance_)
        {
            instance_ = new Registry();
        }

        return *instance_;
    }

  public:
    template <typename T, typename... Args>
        requires IPattern<T>
    static void add(Args &&...args)
    {
        auto pattern = std::make_shared<T>(std::forward<Args>(args)...);

        const auto &id = pattern->getId();
        if (has(id))
        {
            Serial.printf("WARNING: Pattern with ID '%s' already exists! Overwriting it...\n", id.c_str());
        }

        getInstance().instances_[id] = pattern;
    }

    static std::shared_ptr<Pattern> get(const std::string &id)
    {
        std::shared_ptr<Pattern> p = getInstance().instances_.at(id);
        if (!p)
        {
            Serial.printf("ERROR: Pattern with ID '%s' not found!\n", id.c_str());
        }

        return p;
    }

    static bool has(const std::string &id)
    {
        return getInstance().instances_.contains(id);
    }

    static std::vector<std::string> getAllIds()
    {
        std::vector<std::string> ids;
        for (const auto &key : getInstance().instances_ | std::views::keys)
        {
            ids.push_back(key);
        }
        return ids;
    }
};

Registry *Registry::instance_;
