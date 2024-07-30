#include "hook.h"

namespace ClassicParalysis
{
    namespace detail
    {
        void freeze(RE::Actor &a_actor)
        {
            a_actor.PauseCurrentDialogue();
            a_actor.InterruptCast(false);
            a_actor.StopInteractingQuick(true);

            if (const auto currentProcess = a_actor.GetActorRuntimeData().currentProcess)
            {
                currentProcess->ClearMuzzleFlashes();
            }

            a_actor.GetActorRuntimeData().boolFlags.reset(RE::Actor::BOOL_FLAGS::kShouldAnimGraphUpdate);

            if (const auto charController = a_actor.GetCharController(); charController)
            {
                charController->flags.set(RE::CHARACTER_FLAGS::kNotPushable);

                charController->flags.reset(RE::CHARACTER_FLAGS::kRecordHits);
                charController->flags.reset(RE::CHARACTER_FLAGS::kHitFlags);
            }

            a_actor.EnableAI(false);
            a_actor.StopMoving(1.0f);
        }

        void unfreeze(RE::Actor &a_actor)
        {
            a_actor.GetActorRuntimeData().boolFlags.set(RE::Actor::BOOL_FLAGS::kShouldAnimGraphUpdate);

            if (const auto charController = a_actor.GetCharController())
            {
                charController->flags.reset(RE::CHARACTER_FLAGS::kNotPushable);

                charController->flags.set(RE::CHARACTER_FLAGS::kRecordHits);
                charController->flags.set(RE::CHARACTER_FLAGS::kHitFlags);
            }

            a_actor.EnableAI(true);
        }
    }

    namespace Effect
    {
        bool is_invalid_effect(RE::ParalysisEffect *a_this)
        {
            if (const auto spell = a_this->spell)
            {
                if (spell->HasKeywordString("KS_AlduinTimeStop_Key") || spell->HasKeywordString("HoY_SlowTimeKey") || spell->HasKeywordString("SlowTime_CoolKey") || spell->HasKeywordString("Player_SlowTime_Key"))
                {
                    return false;
                }
            }
            return true;
        }

        struct Start
        {
            static void thunk(RE::ParalysisEffect *a_this)
            {
                if (is_invalid_effect(a_this))
                {
                    return func(a_this);
                }

                const auto target = a_this->target;
                const auto ref = target ? target->GetTargetStatsObject() : nullptr;
                const auto actor = ref ? ref->As<RE::Actor>() : nullptr;

                if (actor)
                {
                    if (actor->GetFormID() == SaadiaID)
                    {
                        func(a_this);
                    }
                    else if (!actor->IsDead())
                    {
                        if (!actor->IsPlayerRef())
                        {
                            detail::freeze(*actor);
                        }
                        else
                        {
                            actor->SetLifeState(RE::ACTOR_LIFE_STATE::kRestrained);
                        }
                    }
                }
            }
            static inline REL::Relocation<decltype(thunk)> func;
        };

        struct Finish
        {
            static void thunk(RE::ParalysisEffect *a_this)
            {
                if (is_invalid_effect(a_this))
                {
                    return func(a_this);
                }

                const auto target = a_this->target;
                const auto ref = target ? target->GetTargetStatsObject() : nullptr;
                const auto actor = ref ? ref->As<RE::Actor>() : nullptr;

                if (actor)
                {
                    if (actor->GetFormID() == SaadiaID)
                    {
                        func(a_this);
                    }
                    else
                    {
                        if (!actor->IsPlayerRef())
                        {
                            detail::unfreeze(*actor);
                        }
                        else
                        {
                            actor->SetLifeState(RE::ACTOR_LIFE_STATE::kAlive);
                        }
                        actor->StopMoving(1.0f);
                    }
                }
            }
            static inline REL::Relocation<decltype(thunk)> func;
        };

        void Install()
        {
            Yen::write_vfunc<RE::ParalysisEffect, 0x14, Start>();
            Yen::write_vfunc<RE::ParalysisEffect, 0x15, Finish>();
        }
    }

    namespace Fixes
    {
        struct CanBePushed
        {
            static bool thunk(RE::Actor *a_actor)
            {
                const auto result = func(a_actor);
                if (result && a_actor && !a_actor->IsAIEnabled())
                {
                    detail::unfreeze(*a_actor);
                }
                return result;
            }
            static inline REL::Relocation<decltype(thunk)> func;
        };

        struct GetProcessLevel
        {
            static std::int32_t thunk(RE::Actor *a_actor)
            {
                const auto processLevel = func(a_actor);
                if (a_actor && !a_actor->IsAIEnabled())
                {
                    detail::unfreeze(*a_actor);
                    return -1;
                }
                return processLevel;
            }
            static inline REL::Relocation<decltype(&thunk)> func;
        };

        void Install()
        {
            REL::Relocation<std::uintptr_t> push_actor_away{RELOCATION_ID(38858, 39895), OFFSET(0x7E, 0x68)};
            Yen::write_thunk_call<CanBePushed>(push_actor_away.address());

            REL::Relocation<std::uintptr_t> start_kill_move{RELOCATION_ID(37659, 38613), 0x10};
            Yen::write_thunk_call<GetProcessLevel>(start_kill_move.address());
        }
    }

    void Install()
    {

        Effect::Install();
        Fixes::Install();
    }
}