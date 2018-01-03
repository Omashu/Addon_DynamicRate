#pragma once

#include "../../Common/Interfaces/ModuleInterface.h"
#include "../../Common/Helpers/ModuleHook.hpp"

#include <ATF/CPlayerInfo.hpp>
#include <spdlog/spdlog.h>
#include <vector>
#include <string>
#include <fstream>
#include <filesystem>

#include <rapidjson/prettywriter.h>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>

namespace GameServer
{
    namespace Addon
    {
        class CDynamicRate
            : public Yorozuya::Module::IModule
            , CModuleHook
        {
        public:
			CDynamicRate() { };

			class rate_fld
			{
			public:
				rate_fld(int lv,
					float exp_default,
					float exp_premium) : lv(lv)
					, exp_default(exp_default)
					, exp_premium(exp_premium)
				{};

				int lv;
				float exp_default;
				float exp_premium;
			};

			static std::shared_ptr<spdlog::logger> logger;
			static std::vector<rate_fld*> m_records;
			static bool m_bActivated;

			static float m_fBaseDefaultExpRate;
			static float m_fBasePremiumExpRate;
			static int m_iMaxLv;

            virtual void load() override;

            virtual void unload() override;

            virtual Yorozuya::Module::ModuleName_t get_name() override;

            virtual void configure(const rapidjson::Value& nodeConfig) override;

			static long double calc_base_alter_exp(long double, bool);
			static long double calc_alter_exp(long double, int, bool);

        private:
			static void WINAPIV AlterExp(
				ATF::CPlayer *pObj,
				long double dAlterExp,
				bool bReward,
				bool bUseExpRecoverItem,
				bool bUseExpAdditionItem,
				ATF::Info::CPlayerAlterExp8_ptr next);
        };
    };
};
