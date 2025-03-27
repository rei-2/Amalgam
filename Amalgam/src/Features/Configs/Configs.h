#pragma once
#include "../../SDK/SDK.h"

#include <filesystem>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

class CConfigs
{
	void SaveJson(boost::property_tree::ptree& mapTree, std::string sName, bool bVal);
	void SaveJson(boost::property_tree::ptree& mapTree, std::string sName, int iVal);
	void SaveJson(boost::property_tree::ptree& mapTree, std::string sName, float flVal);
	void SaveJson(boost::property_tree::ptree& mapTree, std::string sName, const IntRange_t& irVal);
	void SaveJson(boost::property_tree::ptree& mapTree, std::string sName, const FloatRange_t& frVal);
	void SaveJson(boost::property_tree::ptree& mapTree, std::string sName, const std::string& sVal);
	void SaveJson(boost::property_tree::ptree& mapTree, std::string sName, const std::vector<std::pair<std::string, Color_t>>& vVal);
	void SaveJson(boost::property_tree::ptree& mapTree, std::string sName, const Color_t& tVal);
	void SaveJson(boost::property_tree::ptree& mapTree, std::string sName, const Gradient_t& tVal);
	void SaveJson(boost::property_tree::ptree& mapTree, std::string sName, const DragBox_t& tVal);
	void SaveJson(boost::property_tree::ptree& mapTree, std::string sName, const WindowBox_t& tVal);

	void LoadJson(boost::property_tree::ptree& mapTree, std::string sName, bool& bVal);
	void LoadJson(boost::property_tree::ptree& mapTree, std::string sName, int& iVal);
	void LoadJson(boost::property_tree::ptree& mapTree, std::string sName, float& flVal);
	void LoadJson(boost::property_tree::ptree& mapTree, std::string sName, IntRange_t& irVal);
	void LoadJson(boost::property_tree::ptree& mapTree, std::string sName, FloatRange_t& frVal);
	void LoadJson(boost::property_tree::ptree& mapTree, std::string sName, std::string& sVal);
	void LoadJson(boost::property_tree::ptree& mapTree, std::string sName, std::vector<std::pair<std::string, Color_t>>& vVal);
	void LoadJson(boost::property_tree::ptree& mapTree, std::string sName, Color_t& tVal);
	void LoadJson(boost::property_tree::ptree& mapTree, std::string sName, Gradient_t& tVal);
	void LoadJson(boost::property_tree::ptree& mapTree, std::string sName, DragBox_t& tVal);
	void LoadJson(boost::property_tree::ptree& mapTree, std::string sName, WindowBox_t& tVal);

public:
	CConfigs();

	bool SaveConfig(const std::string& sConfigName, bool bNotify = true);
	bool LoadConfig(const std::string& sConfigName, bool bNotify = true);
	bool SaveVisual(const std::string& sConfigName, bool bNotify = true);
	bool LoadVisual(const std::string& sConfigName, bool bNotify = true);
	void RemoveConfig(const std::string& sConfigName, bool bNotify = true);
	void RemoveVisual(const std::string& sConfigName, bool bNotify = true);
	void ResetConfig(const std::string& sConfigName, bool bNotify = true);
	void ResetVisual(const std::string& sConfigName, bool bNotify = true);

	boost::property_tree::ptree ColorToTree(const Color_t& color);
	void TreeToColor(const boost::property_tree::ptree& tree, Color_t& out);

	std::string m_sCurrentConfig = "default";
	std::string m_sCurrentVisuals = "default";
	std::string m_sConfigPath;
	std::string m_sVisualsPath;
	std::string m_sCorePath;
	std::string m_sMaterialsPath;
	const std::string m_sConfigExtension = ".json";

	bool m_bConfigLoaded = false;
};

ADD_FEATURE(CConfigs, Configs);