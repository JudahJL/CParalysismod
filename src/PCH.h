#pragma once

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

using namespace std::literals;

namespace logger = SKSE::log;

namespace Yen
{
    using namespace SKSE::stl;

    template <class T>
    void write_thunk_call(std::uintptr_t a_src)
    {
        auto &trampoline = SKSE::GetTrampoline();
        SKSE::AllocTrampoline(14);
        T::func = trampoline.write_call<5>(a_src, T::thunk);
    }

    template <class F, std::size_t idx, class T>
    void write_vfunc()
    {
        REL::Relocation<std::uintptr_t> vtbl{F::VTABLE[0]};
        T::func = vtbl.write_vfunc(idx, T::thunk);
    }
}

#define OFFSET(se, ae) se