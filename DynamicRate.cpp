#include "stdafx.h"

#include "DynamicRate.h"
#include "../../Common/ETypes.h"
#include "../../Common/Helpers/RapidHelper.hpp"

#include <bitset>
#include <ATF/global.hpp>

namespace fs = ::std::experimental::filesystem::v1;

namespace GameServer
{
    namespace Addon
    {
		std::shared_ptr<spdlog::logger> CDynamicRate::logger;
		std::vector<CDynamicRate::rate_fld*> CDynamicRate::m_records;

		float CDynamicRate::m_fBaseDefaultExpRate = 1.0f;
		float CDynamicRate::m_fBasePremiumExpRate = 1.0f;

		int CDynamicRate::m_iMaxLv = 65;

		bool CDynamicRate::m_bActivated = false;

        void CDynamicRate::load()
        {
			enable_hook(&ATF::CPlayer::AlterExp, &CDynamicRate::AlterExp);
        }

		long double CDynamicRate::calc_base_alter_exp(long double alter_exp, bool premium)
		{
			if (premium)
			{
				return alter_exp / m_fBasePremiumExpRate;
			}

			return alter_exp / m_fBaseDefaultExpRate;
		};

		long double CDynamicRate::calc_alter_exp(long double alter_exp, int lv, bool premium)
		{
			const auto& it = std::find_if(m_records.begin(), m_records.end(),
				[&](const rate_fld* fld)
			{
				return lv == fld->lv;
			});

			if (it == m_records.end())
			{
				return alter_exp;
			}

			auto& el = m_records.at(
				std::distance(m_records.begin(), it)
			);

			if (premium)
			{
				return alter_exp * el->exp_premium;
			}

			return alter_exp * el->exp_default;
		};

		void WINAPIV CDynamicRate::AlterExp(
			ATF::CPlayer *pObj,
			long double dAlterExp,
			bool bReward,
			bool bUseExpRecoverItem,
			bool bUseExpAdditionItem,
			ATF::Info::CPlayerAlterExp8_ptr next)
		{
			if (!m_bActivated)
			{
				next(pObj, dAlterExp, bReward, bUseExpRecoverItem, bUseExpAdditionItem);
				return;
			}

			int iLv = pObj->GetLevel();
			bool bPremium = pObj->IsApplyPcbangPrimium();

			if (iLv >= m_iMaxLv)
			{
				next(pObj, .0f, bReward, bUseExpRecoverItem, bUseExpAdditionItem);
				return;
			}

			if (bReward || bUseExpRecoverItem || bUseExpAdditionItem || dAlterExp <= 0) {
				next(pObj, dAlterExp, bReward, bUseExpRecoverItem, bUseExpAdditionItem);
				return;
			}

			long double dNewAlterExp = calc_alter_exp(
				calc_base_alter_exp(dAlterExp, bPremium),
				iLv,
				bPremium
			);

			next(pObj, dNewAlterExp, bReward, bUseExpRecoverItem, bUseExpAdditionItem);
		};

        void CDynamicRate::unload()
        {
            cleanup_all_hook();
        }

        Yorozuya::Module::ModuleName_t CDynamicRate::get_name()
        {
            static const Yorozuya::Module::ModuleName_t name = "addon.dynamic_rate";
            return name;
        }

        void CDynamicRate::configure(
            const rapidjson::Value& nodeConfig)
        {
			m_bActivated = RapidHelper::GetValueOrDefault(nodeConfig, "activated", false);
			bool m_bFlushLogs = RapidHelper::GetValueOrDefault(nodeConfig, "flush_logs", true);
			std::string sPathToConfig = RapidHelper::GetValueOrDefault<std::string>(nodeConfig, "config_path", "./YorozuyaGS/dynamic_rate.json");

			// logger instance
			logger = spdlog::basic_logger_mt(get_name(), "YorozuyaGS/Logs/DynamicRate.txt");

			if (m_bFlushLogs)
			{
				logger->flush();
			}

			logger->info("configure...");

			if (!m_bActivated)
			{
				logger->info("addon is disabled");
				return;
			}

			m_iMaxLv = RapidHelper::GetValueOrDefault<int>(nodeConfig, "max_lv", m_iMaxLv);
			m_fBaseDefaultExpRate = RapidHelper::GetValueOrDefault<float>(nodeConfig, "base_default_exp_rate", m_fBaseDefaultExpRate);
			m_fBasePremiumExpRate = RapidHelper::GetValueOrDefault<float>(nodeConfig, "base_premium_exp_rate", m_fBasePremiumExpRate);

			logger->info("set base rates default / premium: {} / {}", m_fBaseDefaultExpRate, m_fBasePremiumExpRate);
			logger->info("set max lv: {}", m_iMaxLv);
			logger->info("try to loading level rates {}...", sPathToConfig);

			if (!fs::exists(sPathToConfig))
			{
				logger->info("configuration file not found, addon force disabled");
				m_bActivated = false;
				return;
			}

			std::ifstream ifs(sPathToConfig);
			rapidjson::IStreamWrapper isw(ifs);
			rapidjson::Document Config;

			if (Config.ParseStream(isw).HasParseError())
			{
				logger->critical("config file - corrupted, addon force disabled");
				m_bActivated = false;
				return;
			}

			for (auto& rec : Config.GetArray())
			{
				if (!rec.HasMember("lv") || !rec["lv"].IsInt())
				{
					continue;
				}

				if (rec["lv"].GetInt() <= 0 || rec["lv"].GetInt() > m_iMaxLv)
				{
					continue;
				}

				float fBaseExpRate = (rec.HasMember("default_exp_rate")
					&& rec["default_exp_rate"].IsFloat()) ? rec["default_exp_rate"].GetFloat() : 1.0f;

				float fPremiumExpRate = (rec.HasMember("premium_exp_rate")
					&& rec["premium_exp_rate"].IsFloat()) ? rec["premium_exp_rate"].GetFloat() : fBaseExpRate;

				rate_fld *fld = new rate_fld(rec["lv"].GetInt(),
					fBaseExpRate,
					fPremiumExpRate);

				m_records.push_back(fld);

				logger->info("created fld lv: {}, default_exp_rate: {}, premium_exp_rate: {}",
					rec["lv"].GetInt(),
					fBaseExpRate,
					fPremiumExpRate);
			}
        }

    }
}