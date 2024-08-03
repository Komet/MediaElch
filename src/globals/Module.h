#pragma once

#include <QString>

namespace mediaelch {
namespace core {

class ModuleInterface
{
public:
    ModuleInterface();
    virtual ~ModuleInterface();

    virtual QString moduleName() = 0;

    virtual void onInit() {}
};

} // namespace core
} // namespace mediaelch
