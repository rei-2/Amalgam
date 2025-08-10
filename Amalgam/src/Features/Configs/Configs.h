#pragma once
#include "../../SDK/SDK.h"

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <filesystem>

class CConfigs
{
public:
	CConfigs();

	bool SaveConfig(const std::string& sConfigName, bool bNotify = true);
	bool LoadConfig(const std::string& sConfigName, bool bNotify = true);
	void DeleteConfig(const std::string& sConfigName, bool bNotify = true);
	void ResetConfig(const std::string& sConfigName, bool bNotify = true);
	bool SaveVisual(const std::string& sConfigName, bool bNotify = true);
	bool LoadVisual(const std::string& sConfigName, bool bNotify = true);
	void DeleteVisual(const std::string& sConfigName, bool bNotify = true);
	void ResetVisual(const std::string& sConfigName, bool bNotify = true);

	void SaveJson(boost::property_tree::ptree& t, const std::string& s, bool v);
	void SaveJson(boost::property_tree::ptree& t, const std::string& s, byte v);
	void SaveJson(boost::property_tree::ptree& t, const std::string& s, int v);
	void SaveJson(boost::property_tree::ptree& t, const std::string& s, float v);
	void SaveJson(boost::property_tree::ptree& t, const std::string& s, const std::string& v);
	void SaveJson(boost::property_tree::ptree& t, const std::string& s, const IntRange_t& v);
	void SaveJson(boost::property_tree::ptree& t, const std::string& s, const FloatRange_t& v);
	void SaveJson(boost::property_tree::ptree& t, const std::string& s, const std::vector<std::pair<std::string, Color_t>>& v);
	void SaveJson(boost::property_tree::ptree& t, const std::string& s, const Color_t& v);
	void SaveJson(boost::property_tree::ptree& t, const std::string& s, const Gradient_t& v);
	void SaveJson(boost::property_tree::ptree& t, const std::string& s, const DragBox_t& v);
	void SaveJson(boost::property_tree::ptree& t, const std::string& s, const WindowBox_t& v);
	void SaveJson(boost::property_tree::ptree& t, const std::string& s, const Chams_t& v);
	void SaveJson(boost::property_tree::ptree& t, const std::string& s, const Glow_t& v);

	void LoadJson(const boost::property_tree::ptree& t, const std::string& s, bool& v);
	void LoadJson(const boost::property_tree::ptree& t, const std::string& s, byte& v);
	void LoadJson(const boost::property_tree::ptree& t, const std::string& s, int& v);
	void LoadJson(const boost::property_tree::ptree& t, const std::string& s, float& v);
	void LoadJson(const boost::property_tree::ptree& t, const std::string& s, std::string& v);
	void LoadJson(const boost::property_tree::ptree& t, const std::string& s, IntRange_t& v);
	void LoadJson(const boost::property_tree::ptree& t, const std::string& s, FloatRange_t& v);
	void LoadJson(const boost::property_tree::ptree& t, const std::string& s, std::vector<std::pair<std::string, Color_t>>& v);
	void LoadJson(const boost::property_tree::ptree& t, const std::string& s, Color_t& v);
	void LoadJson(const boost::property_tree::ptree& t, const std::string& s, Gradient_t& v);
	void LoadJson(const boost::property_tree::ptree& t, const std::string& s, DragBox_t& v);
	void LoadJson(const boost::property_tree::ptree& t, const std::string& s, WindowBox_t& v);
	void LoadJson(const boost::property_tree::ptree& t, const std::string& s, Chams_t& v);
	void LoadJson(const boost::property_tree::ptree& t, const std::string& s, Glow_t& v);
	std::string m_sCurrentConfig = "default";
	std::string m_sCurrentVisuals = "default";
	std::string m_sConfigPath;
	std::string m_sVisualsPath;
	std::string m_sCorePath;
	std::string m_sMaterialsPath;
	const std::string m_sConfigExtension = ".json";
};

ADD_FEATURE(CConfigs, Configs);