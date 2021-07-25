#pragma once
#include "dbcomid.hpp"
#include "monster.hpp"

class RebornZombie final: public Monster
{
    private:
        bool m_standMode = false;

    public:
        RebornZombie(uint32_t monID, ServerMap *mapPtr, int argX, int argY, int argDir)
            : Monster(monID, mapPtr, argX, argY, argDir, 0)
        {}

    protected:
        corof::long_jmper updateCoroFunc() override;

    protected:
        ActionNode makeActionStand() const override
        {
            return ActionStand
            {
                .x = X(),
                .y = Y(),
                .direction = Direction(),
                .extParam
                {
                    .rebornZombie
                    {
                        .standMode = m_standMode,
                    },
                },
            };
        }

        void setStandMode(bool standMode)
        {
            if(standMode != m_standMode){
                m_standMode = standMode;
                dispatchAction(ActionTransf
                {
                    .x = X(),
                    .y = Y(),

                    .direction = Direction(),
                    .extParam
                    {
                        .rebornZombie
                        {
                            .standModeReq = standMode,
                        }
                    },
                });
            }
        }

    protected:
        bool struckDamage(const DamageNode &damage)
        {
            if(!m_standMode){
                switch(damage.magicID){
                    case DBCOM_MAGICID(u8"火墙"):
                    case DBCOM_MAGICID(u8"地狱火"):
                    case DBCOM_MAGICID(u8"冰沙掌"):
                        {
                            setStandMode(true);
                            break;
                        }
                    default:
                        {
                            break;
                        }
                }
                return true;
            }

            if(damage){
                m_HP = (std::max<int>)(0, HP() - damage.damage);
                dispatchHealth();

                if(HP() <= 0){
                    goDie();
                }
                return true;
            }
            return false;
        }
};
